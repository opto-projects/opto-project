#ifndef CAMERA_WORKER
#define CAMERA_WORKER


#include "util/OriConfigDlg.h"
#include "util/OriDialogs.h"
#include "util/OriLayouts.h"
//#include "helpers/OriWidgets.h"
//#include "tools/OriSettings.h"
#include "util/OriValueEdit.h"
#include "settings.h"
#include "acquisitionsystem.h"
#include "beam/beam_calc.h"
#include "MeasureSaver.h"


#include <QApplication>
#include <QDebug>
#include <QMutex>
#include <QQueue>
#include <QThread>
#include <QtMath>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QSettings>
#include <QDateTime>
#include <QElapsedTimer>
#include <QVector>

#define PLOT_FRAME_DELAY_MS 200
#define STAT_DELAY_MS 1000
#define MEASURE_BUF_SIZE 1000
#define MEASURE_BUF_COUNT 2
#define SQR(x) ((x)*(x))



enum MeasureDataCol { COL_BRIGHTNESS, COL_POWER, COL_DEBUG_1, COL_DEBUG_2 };

class CameraWorker
{
    friend class AcquisitionSystem;
protected:
    CgnBeamCalc c;
    CgnBeamResult r;
    CgnBeamBkgnd g;

    AcquisitionSystem* camera;
    QThread *thread;
    std::atomic_bool rawView = false;

    QDateTime start;
    QElapsedTimer timer;
    qint64 tm;
    qint64 prevFrame = 0;
    qint64 prevReady = 0;
    qint64 prevStat = 0;
    qint64 prevSaveImg = 0;
    double avgFrameCount = 0;
    double avgFrameTime = 0;
    double avgAcqTime = 0;
    double avgCalcTime = 0;

    QMutex cfgMutex;
    bool subtract;
    bool normalize;
    bool fullRange;
    bool useRoi;
    bool multiRoi;
    bool reconfig = false;
    double powerScale = 0;
    RoiRect roi;
    QList<RoiRect> rois;
    double *graph = nullptr;
    QVector<double> subtracted;
    QVector<CgnBeamResult> results;
    QVector<QQueue<CgnBeamResult>> mavgs;
    QVector<CgnBeamResult> sdevs;

    MeasureSaver* saver = nullptr;
    QMutex saverMutex;
    QVector<Measurement> measurBuf1;
    QVector<Measurement> measurBuf2;
    Measurement *measurBufs[MEASURE_BUF_COUNT];
    Measurement *measurs;
    int measurIdx = 0;
    int measurBufIdx = 0;
    qint64 measureStart = 0;
    qint64 measureDuration = -1;
    qint64 saveImgInterval = 0;
    QObject *rawImgRequest = nullptr;
    QObject *brightRequest = nullptr;
    QObject *expWarningRequest = nullptr;
    double brightness = 0;
    bool showBrightness = false;
    bool saveBrightness = false;
    bool showPower = false;
    bool hasPowerWarning = false;
    int calibratePowerFrames = 0;
    double calibratePowerTotal = 0;
    int powerDecimalFactor = 0;
    double power = 0;
    double powerSdev = 0;
    bool doMavg = false;
    int mavgFrames = 0;

    QMap<QString, QVariant> stats;

    std::function<QMap<int, CamTableData>()> tableData;

    int framesErr = 0;
    int framesDropped = 0;
    int framesUnderrun = 0;
    int framesIncomplete = 0;

    const char *logId;

    QString _configGroup;

    enum ConfigPages { cfgPlot, cfgTable, cfgBgnd, cfgCentr, cfgRoi, cfgMax };

public:
    CameraWorker();
    virtual ~CameraWorker();

    void setup(AcquisitionSystem* cam)
    {
        camera = cam;

        c.w = camera->width();
        c.h = camera->height();
        c.bpp = camera->bpp();
       
        if (graph != nullptr) {
            delete[] graph;
            graph = nullptr; 
        }

        configure();
        initTableData();

        graph = new double[c.w * c.h]();

        showBrightness = true;
        saveBrightness = true;

        togglePowerMeter();

    }


    void initTableData(){
        tableData = [this] {
            QMap<int, CamTableData> data = {};
            if (showBrightness)
                data[ROW_BRIGHTNESS] = { brightness, CamTableData::VALUE3 };
            if (showPower)
                data[ROW_POWER] = {
                    QVariantList{
                        power * powerScale,
                        powerSdev * powerScale,
                        powerDecimalFactor,
                    },
                    CamTableData::POWER,
                    hasPowerWarning
            };
            return data;
        };
    }

    const BeamConfig& beamConfig() const { return _config; }

    void configure()
    {
        reconfig = false;

        memset(&r, 0, sizeof(CgnBeamResult));
        memset(&g, 0, sizeof(CgnBeamBkgnd));

        auto cfg = beamConfig();
        g.max_iter = cfg.bgnd.iters;
        g.precision = cfg.bgnd.precision;
        g.corner_fraction = cfg.bgnd.corner;
        g.nT = cfg.bgnd.noise;
        g.mask_diam = cfg.bgnd.mask;
        if ( cfg.roi.isValid()) {
            g.ax1 = qRound(cfg.roi.left * double(c.w));
            g.ay1 = qRound(cfg.roi.top * double(c.h));
            g.ax2 = qRound(cfg.roi.right * double(c.w));
            g.ay2 = qRound(cfg.roi.bottom * double(c.h));
            r.x1 = g.ax1;
            r.y1 = g.ay1;
            r.x2 = g.ax2;
            r.y2 = g.ay2;
        }
        else {
            g.ax2 = c.w;
            g.ay2 = c.h;
            r.x2 = c.w;
            r.y2 = c.h;
        }
        subtract = cfg.bgnd.on;
        if (subtract) {
            subtracted = QVector<double>(camera->width()  * camera->height());
            g.subtracted = subtracted.data();
        }
        normalize = cfg.plot.normalize;
        fullRange = cfg.plot.fullRange;
        multiRoi = cfg.roiMode == ROI_MULTI;
        useRoi = cfg.roiMode != ROI_NONE;
        roi = cfg.roi;
        rois = cfg.rois;
        results.resize(multiRoi ? rois.size() : 1);
        g.subtract_bkgnd_v = multiRoi ? 1 : 0;

        doMavg = cfg.mavg.on;
        mavgFrames = cfg.mavg.frames;
        if (doMavg) {
            mavgs.resize(multiRoi ? rois.size() : 1);
            sdevs.resize(multiRoi ? rois.size() : 1);
        } else {
            mavgs.clear();
            sdevs.clear();
        }
    }

    void reconfigure()
    {
        cfgMutex.lock();
        reconfig = true;
        cfgMutex.unlock();
    }

    void checkReconfig()
    {
        cfgMutex.lock();
        if (reconfig) {
            configure();
            qDebug() << logId << "Reconfigured";
        }
        cfgMutex.unlock();
    }

    inline void cgnConvert(uchar* buf, int memorySize)
    {
        
        if (c.bpp == 12)
            cgn_convert_12g24_to_u16(c.buf, buf, memorySize);
        else if (c.bpp == 10)
            cgn_convert_10g40_to_u16(c.buf, buf, memorySize);
        else
            c.buf = buf;
    }

    inline void startAcqTime()
    {
        start = QDateTime::currentDateTime();
        timer.start();
    }

    inline void prevFrameAcqTime()
    {
        this->tm = this->timer.elapsed();
        this->avgFrameCount++;
        this->avgFrameTime += this->tm - this->prevFrame;
        this->prevFrame = this->tm;
        this->tm = this->timer.elapsed();
    }

    inline void frameErrors()
    {
        this->framesErr++;
        this->stats[QStringLiteral("frameErrors")] = this->framesErr;
        QString errKey = QStringLiteral("frameError_") + QString::number(1, 16);
        this->stats[errKey] = this->stats[errKey].toInt() + 1;
    }

    inline void markAcqTime()
    {
        avgAcqTime = avgAcqTime * 0.9 + (timer.elapsed() - tm) * 0.1;

        this->tm = this->timer.elapsed();
    }

    inline void markCalcTime()
    {
        avgCalcTime = avgCalcTime * 0.9 + (timer.elapsed() - tm) * 0.1;
        this->tm = this->timer.elapsed();
    }


    inline CameraStats finishFreme()
    {

        if (this->tm - this->prevStat >= STAT_DELAY_MS) {
            this->prevStat = this->tm;

            if (1) {
                framesDropped = 0;
                framesUnderrun = framesUnderrun + 1;
                framesIncomplete = 0;
                stats[QStringLiteral("framesDropped")] = framesDropped;
                stats[QStringLiteral("framesUnderrun")] = framesUnderrun;
                stats[QStringLiteral("framesIncomplete")] = framesIncomplete;
            }

            double ft = avgFrameTime / avgFrameCount;
            avgFrameTime = 0;
            avgFrameCount = 0;
            CameraStats st;
            {
                st.fps = 1000.0 / ft;
                st.hardFps = 1000.0 / ft;
                st.measureTime = measureStart > 0 ? timer.elapsed() - measureStart : -1;
            };
            return st;
        }
        return CameraStats();
    }

    bool canMavg() const { return true; }

    inline void calcMavg(int roiIndex)
    {
        CgnBeamResult avg;
        memset(&avg, 0, sizeof(avg));
        QQueue<CgnBeamResult> &q = mavgs[roiIndex];
        q.enqueue(r);
        if (q.size() > mavgFrames) {
            const CgnBeamResult first = q.dequeue();
            const CgnBeamResult &last = r;
            const CgnBeamResult &prev = results.at(roiIndex);
            const double c = q.size();
            avg.xc = prev.xc + (last.xc - first.xc) / c;
            avg.yc = prev.yc + (last.yc - first.yc) / c;
            avg.dx = prev.dx + (last.dx - first.dx) / c;
            avg.dy = prev.dy + (last.dy - first.dy) / c;
            avg.phi = prev.phi + (last.phi - first.phi) / c;
            avg.p = prev.p + (last.p - first.p) / c;
        } else {
            for (const auto &r : q) {
                avg.xc += r.xc;
                avg.yc += r.yc;
                avg.dx += r.dx;
                avg.dy += r.dy;
                avg.phi += r.phi;
                avg.p += r.p;
            }
            const double c = q.size();
            avg.xc /= c;
            avg.yc /= c;
            avg.dx /= c;
            avg.dy /= c;
            avg.phi /= c;
            avg.p /= c;
        }
        results[roiIndex] = avg;

        CgnBeamResult sdev;
        memset(&sdev, 0, sizeof(sdev));
        for (const auto &r : q) {
            sdev.xc += SQR(r.xc - avg.xc);
            sdev.yc += SQR(r.yc - avg.yc);
            sdev.dx += SQR(r.dx - avg.dx);
            sdev.dy += SQR(r.dy - avg.dy);
            sdev.phi += SQR(r.phi - avg.phi);
            sdev.p += SQR(r.p - avg.p);
        }
        const double c = q.size();
        sdev.xc = qSqrt(sdev.xc / c);
        sdev.yc = qSqrt(sdev.yc / c);
        sdev.dx = qSqrt(sdev.dx / c);
        sdev.dy = qSqrt(sdev.dy / c);
        sdev.phi = qSqrt(sdev.phi / c);
        sdev.p = qSqrt(sdev.p / c);
        sdevs[roiIndex] = sdev;
    }

    inline void calcResult()
    {
        power = 0;
        powerSdev = 0;

        if (!rawView) {
            if (multiRoi) {
                if (subtract) {
                    g.min = 1e10;
                    g.max = -1e10;
                    cgn_copy_to_f64(&c, g.subtracted, nullptr);
                }
                for (int i = 0; i < rois.size(); i++) {
                    setRoi(rois.at(i));
                    cgn_calc_beam_bkgnd(&c, &g, &r);
                    if (doMavg) {
                        calcMavg(i);
                    } else {
                        results[i] = r;
                    }
                    if (showPower) {
                        power += results.at(i).p;
                        if (doMavg)
                            powerSdev += sdevs.at(i).p;
                    }
                }
                if (showPower) {
                    power /= double(rois.size());
                    if (doMavg)
                        powerSdev /= double(rois.size());
                }
            } 
            else {
                setRoi(roi);
                cgn_calc_beam_bkgnd(&c, &g, &r);
                if (doMavg) {
                    calcMavg(0);
                } else {
                    results[0] = r;
                }
                if (showPower) {
                    power = results.at(0).p;
                    if (doMavg)
                        powerSdev = sdevs.at(0).p;
                }
            }
        }



        saverMutex.lock();
        if (calibratePowerFrames > 0) {
            qDebug() << logId << "calibrate power"
                 << "| step =" << calibratePowerFrames
                 << "| digital_intensity =" << power;
            calibratePowerTotal += power;
            if (--calibratePowerFrames == 0) {
                hasPowerWarning = false;
                calibratePowerTotal /= double(beamConfig().power.avgFrames);
                powerScale = beamConfig().power.power / calibratePowerTotal;
                qDebug() << logId << "calibrate power"
                    << "| digital_intensity_avg =" << calibratePowerTotal
                    << "| power =" << beamConfig().power.power
                    << "| scale =" << powerScale;
            }
        }
        if (rawImgRequest) {
            auto e = new ImageEvent();
            e->time = 0;
            e->buf = QByteArray((const char*)c.buf, c.w*c.h*(c.bpp > 8 ? 2 : 1));
            QCoreApplication::postEvent(rawImgRequest, e);
            rawImgRequest = nullptr;
        }
        if (brightRequest) {
            auto e = new BrightEvent;
            e->level = cgn_calc_brightness_1(&c);
            QCoreApplication::postEvent(brightRequest, e);
            brightRequest = nullptr;
        }
        if (expWarningRequest ) {
            auto e = new ExpWarningEvent;
            e->overexposed = cgn_calc_overexposure(&c, 0.8);
            QCoreApplication::postEvent(expWarningRequest, e);
            expWarningRequest = nullptr;
        }
        if (!rawView ) {
            qint64 time = timer.elapsed();
            if (saveImgInterval > 0 && (prevSaveImg == 0 || time - prevSaveImg >= saveImgInterval)) {
                prevSaveImg = time;
                auto e = new ImageEvent;
                e->time = time;
                e->buf = QByteArray((const char*)c.buf, c.w*c.h*(c.bpp > 8 ? 2 : 1));
                QCoreApplication::postEvent(saver, e);
            }
            measurs->time = time;
            if (multiRoi) {
                for (int i = 0; i < results.size(); i++) {
                    const auto &r = results.at(i);
                    measurs->cols[MULTIRES_IDX_NAN(i)] = r.nan ? 1 : 0;
                    measurs->cols[MULTIRES_IDX_DX(i)] = r.dx;
                    measurs->cols[MULTIRES_IDX_DY(i)] = r.dy;
                    measurs->cols[MULTIRES_IDX_XC(i)] = r.xc;
                    measurs->cols[MULTIRES_IDX_YC(i)] = r.yc;
                    measurs->cols[MULTIRES_IDX_PHI(i)] = r.phi;
                }
            } else {
                measurs->nan = r.nan;
                measurs->dx = r.dx;
                measurs->dy = r.dy;
                measurs->xc = r.xc;
                measurs->yc = r.yc;
                measurs->phi = r.phi;
            }
            if (saveBrightness)
                measurs->cols[COL_BRIGHTNESS] = cgn_calc_brightness_1(&c);
            if (showPower && calibratePowerFrames == 0)
                measurs->cols[COL_POWER] = power * powerScale;
            if (++measurIdx == MEASURE_BUF_SIZE ||
                (measureDuration > 0 && (time - measureStart >= measureDuration))) {
                sendMeasure();
            } else {
                measurs++;
            }
        }
        saverMutex.unlock();
    }


    inline void sendMeasure()
    {
        auto e = new MeasureEvent;
        e->num = measurBufIdx;
        e->count = measurIdx;
        e->results = measurBufs[measurBufIdx % MEASURE_BUF_COUNT];
        e->stats = stats;
        QCoreApplication::postEvent(saver, e);
        measurs = measurBufs[++measurBufIdx % MEASURE_BUF_COUNT];
        measurIdx = 0;
    }

    void startMeasureSaver(MeasureSaver* s)
    {
        saverMutex.lock();
        measurIdx = 0;
        measurBufIdx = 0;
        measurs = measurBufs[0];
        measureStart = timer.elapsed();
        measureDuration = s->config().durationInf ? -1 : s->config().durationSecs() * 1000;
        saveImgInterval = s->config().saveImg ? s->config().imgIntervalSecs() * 1000 : 0;
        saver = s;
        saver->setCaptureStart(start);
        saverMutex.unlock();
    }

    void stopMeasureSaver()
    {
        saverMutex.lock();
        if (measurIdx > 0)
            sendMeasure();
        saver = nullptr;
        measureStart = -1;
        measureDuration = -1;
        saverMutex.unlock();
    }

    std::function<void(const QVector<CgnBeamResult>& val, const QVector<CgnBeamResult>& sdev, const QMap<int, CamTableData>& data)> setTableData;
    std::function<void(const QVector<CgnBeamResult>& r, double min, double max)> setPlotData;

    inline bool showResults()
    {
        const double rangeTop = (1 << c.bpp) - 1;

        if (showBrightness)
            brightness = cgn_calc_brightness_1(&c);

        if (rawView)
        {
            cgn_copy_to_f64(&c, graph, &g.max);
            //plot->invalidateGraph();
            //plot->setResult({}, 0, rangeTop);
            //table->setResult({}, {}, tableData());
            setTableData({}, {}, tableData());
            setPlotData({}, 0, rangeTop);
            return true;
        }

        double minZ, maxZ;
        cgn_ext_copy_to_f64(&c, &g, graph, normalize, fullRange, &minZ, &maxZ);

        if (setTableData&& setPlotData)
        {
            setTableData(results, sdevs, tableData());
            setPlotData(results, minZ, maxZ);
        } 
        //plot->invalidateGraph();
        //plot->setResult(results, minZ, maxZ);

        //table->setResult(results, sdevs, tableData());
        return true;
    }

    void requestRawImg(QObject *sender)
    {
        saverMutex.lock();
        rawImgRequest = sender;
        saverMutex.unlock();
    }

    void requestBrightness(QObject *sender)
    {
        saverMutex.lock();
        brightRequest = sender;
        saverMutex.unlock();
    }

    void requestExpWarning(QObject *sender)
    {
        saverMutex.lock();
        expWarningRequest = sender;
        saverMutex.unlock();
    }

    void setRawView(bool on, bool reconfig)
    {
        saverMutex.lock();
        if (rawView != on) {
            rawView = on;
            if (reconfig)
                this->reconfig = true;
        }
        saverMutex.unlock();
    }

    void togglePowerMeter() {
        saverMutex.lock();
        showPower = beamConfig().power.on;
        if (showPower) {
            powerDecimalFactor = beamConfig().power.decimalFactor;
            calibratePowerTotal = 0;
            calibratePowerFrames = std::clamp(beamConfig().power.avgFrames, PowerMeter::minAvgFrames, PowerMeter::maxAvgFrames);
        }
        saverMutex.unlock();
    }

    virtual bool isPowerMeter() const { return true; }

    void raisePowerWarning()  {  hasPowerWarning = true; }

    bool setupPowerMeter()
    {
        auto cbEnable = new QCheckBox(qApp->tr("Show power"));
        auto seFrames = new QSpinBox; seFrames->setRange(1, 10);
        auto edPower = new ValueEdit;
        auto cbFactor = new QComboBox;
        cbFactor->addItem(qApp->tr("W"));
        cbFactor->addItem(qApp->tr("mW"));
        cbFactor->addItem(qApp->tr("uW"));
        cbFactor->addItem(qApp->tr("nW"));

        cbEnable->setChecked(_config.power.on);
        seFrames->setValue(_config.power.avgFrames);
        edPower->setValue(_config.power.power);
        cbFactor->setCurrentIndex(_config.power.decimalFactor / -3);

        auto w = LayoutV({
            cbEnable,
            SpaceV(1),
            qApp->tr("Current brightness\naveraged over frames:"),
            seFrames,
            SpaceV(1),
            qApp->tr("Corresponds to power:"),
            LayoutH({edPower, cbFactor}),
            }).setMargin(0).makeWidgetAuto();
        bool ok = Dialog(w)
            .withHelpIcon(":/ori_images/help")
            //.withOnHelp([]{ HelpSystem::topic("power_meter"); })
            .withContentToButtonsSpacingFactor(3)
            .windowModal()
            .exec();
        if (ok) {
            _config.power.on = cbEnable->isChecked();
            _config.power.avgFrames = std::clamp(seFrames->value(), PowerMeter::minAvgFrames, PowerMeter::maxAvgFrames);
            _config.power.power = qMax(edPower->value(), 0.0);
            _config.power.decimalFactor = cbFactor->currentIndex() * -3;
            saveConfig(1);
            togglePowerMeter();
        }
        return ok;
    }

    void loadConfig()
    {
        QSettings*  s = Settings::getInstance()->getQSettings();
        s->beginGroup(_configGroup);
        _config.load(s);
        loadConfigMore(s);
        s->endGroup();
    }

    void saveConfig(bool saveMore = true)
    {
        QSettings* s = Settings::getInstance()->getQSettings();
        s->beginGroup(_configGroup);
        _config.save(s);
        if (saveMore)
            saveConfigMore(s);
        s->endGroup();
    }

    bool editConfig(int page)
    {
        ConfigDlgOpts opts;
        opts.currentPageId = page;
        opts.objectName = "CamConfigDlg";
        opts.pageIconSize = 32;
        opts.pages = {
            ConfigPage(cfgPlot, qApp->tr("Plot"), ":/toolbar/zoom_sensor")
                .withHelpTopic("cam_settings_plot")
        };
        if (canMavg()) {
            opts.pages << ConfigPage(cfgTable, qApp->tr("Table"), ":/toolbar/table")
                .withSpacing(12)
                .withHelpTopic("cam_settings_table");
        }
        opts.pages
            << ConfigPage(cfgBgnd, qApp->tr("Background"), ":/toolbar/beam")
            .withSpacing(12)
            .withHelpTopic("cam_settings_bgnd")
            << ConfigPage(cfgCentr, qApp->tr("Centroid"), ":/toolbar/centroid")
            .withSpacing(12)
            .withLongTitle(qApp->tr("Centroid Calculation"))
            .withHelpTopic("cam_settings_centr")
            << ConfigPage(cfgRoi, qApp->tr("ROI"), ":/toolbar/roi")
            .withSpacing(12)
            .withLongTitle(qApp->tr("Region of Interest"))
            .withHelpTopic("cam_settings_roi")
            ;
        //opts.onHelpRequested = [](const QString &topic){ HelpSystem::topic(topic); };
        auto hardScale = camera->sensorScale();
        bool useSensorScale = !_config.plot.customScale.on;
        bool useCustomScale = _config.plot.customScale.on;
        bool scaleFullRange = _config.plot.fullRange;
        bool scaleDataRange = !_config.plot.fullRange;
        double cornerFraction = _config.bgnd.corner * 100;
        int roiPixelLeft = qRound(double(camera->width()) * _config.roi.left);
        int roiPixelRight = qRound(double(camera->width()) * _config.roi.right);
        int roiPixelTop = qRound(double(camera->height()) * _config.roi.top);
        int roiPixelBottom = qRound(double(camera->height()) * _config.roi.bottom);
        bool roiOn = _config.roiMode == ROI_SINGLE;
        bool oldRoiOn = roiOn;
        opts.items = {
            new ConfigItemBool(cfgPlot, qApp->tr("Normalize data"), &_config.plot.normalize),
            new ConfigItemSpace(cfgPlot, 12),
            (new ConfigItemBool(cfgPlot, qApp->tr("Colorize over full range"), &scaleFullRange))
                ->withRadioGroup("colorize"),
            (new ConfigItemBool(cfgPlot, qApp->tr("Colorize over data range"), &scaleDataRange))
                ->withRadioGroup("colorize"),
            new ConfigItemSpace(cfgPlot, 12),
            new ConfigItemBool(cfgPlot, qApp->tr("Rescale pixels"), &_config.plot.rescale),
            (new ConfigItemBool(cfgPlot, qApp->tr("Use hardware scale"), &useSensorScale))
                ->setDisabled(!hardScale.on)
                ->withRadioGroup("pixel_scale")
                ->withHint(hardScale.on
                    ? QString("1px = %1 %2").arg(hardScale.factor).arg(hardScale.unit)
                    : qApp->tr("AcquisitionSystem does not provide meta data")),
            (new ConfigItemBool(cfgPlot, qApp->tr("Use custom scale"), &useCustomScale))
                ->withRadioGroup("pixel_scale"),
            new ConfigItemReal(cfgPlot, qApp->tr("1px equals to"), &_config.plot.customScale.factor),
            (new ConfigItemStr(cfgPlot, qApp->tr("of units"), &_config.plot.customScale.unit))
                ->withAlignment(Qt::AlignRight)
        };
        if (canMavg()) {
            opts.items
                << new ConfigItemBool(cfgTable, qApp->tr("Show moving average"), &_config.mavg.on)
                << (new ConfigItemInt(cfgTable, qApp->tr("Over number of frames"), &_config.mavg.frames))
                ->withMinMax(2, 100)
                ;
        }
        opts.items
            << new ConfigItemBool(cfgBgnd, qApp->tr("Subtract background"), &_config.bgnd.on)
            << (new ConfigItemReal(cfgBgnd, qApp->tr("Corner Fraction %"), &cornerFraction))
            ->withHint(qApp->tr("ISO 11146 recommends 2-5%"), false)
            << (new ConfigItemReal(cfgBgnd, qApp->tr("Noise Factor"), &_config.bgnd.noise))
            ->withHint(qApp->tr("ISO 11146 recommends 2-4"), false)

            << (new ConfigItemReal(cfgCentr, qApp->tr("Mask Diameter"), &_config.bgnd.mask))
            ->withHint(qApp->tr("ISO 11146 recommends 3"), false)
            << (new ConfigItemInt(cfgCentr, qApp->tr("Max Iterations"), &_config.bgnd.iters))
            ->withMinMax(0, 50)
            << new ConfigItemReal(cfgCentr, qApp->tr("Precision"), &_config.bgnd.precision)

            << (new ConfigItemBool(cfgRoi, qApp->tr("Use region"), &roiOn))
            ->withHint(qApp->tr(
                "These are raw pixels values. "
                "Use the menu command <b>AcquisitionSystem ► Edit ROI</b> "
                "to change region interactively in scaled units"), true)
            << (new ConfigItemInt(cfgRoi, qApp->tr("Left"), &roiPixelLeft))
            ->withMinMax(0, camera->width())
            << (new ConfigItemInt(cfgRoi, qApp->tr("Top"), &roiPixelTop))
            ->withMinMax(0, camera->height())
            << (new ConfigItemInt(cfgRoi, qApp->tr("Right"), &roiPixelRight))
            ->withMinMax(0, camera->width())
            << (new ConfigItemInt(cfgRoi, qApp->tr("Bottom"), &roiPixelBottom))
            ->withMinMax(0, camera->height())
            ;
        initConfigMore(opts);
        if (ConfigDlg::edit(opts))
        {
            RoiRect oldRoi = _config.roi;
            _config.plot.fullRange = scaleFullRange;
            _config.plot.customScale.on = useCustomScale;
            _config.bgnd.corner = cornerFraction / 100.0;
            _config.roi.left = double(roiPixelLeft) / double(camera->width());
            _config.roi.right = double(roiPixelRight) / double(camera->width());
            _config.roi.top = double(roiPixelTop) / double(camera->height());
            _config.roi.bottom = double(roiPixelBottom) / double(camera->height());
            _config.roi.fix();
            if (oldRoiOn != roiOn) {
                _config.roiMode = roiOn ? ROI_SINGLE : ROI_NONE;
            }
            saveConfig(false);
            if (!oldRoi.isEqual(_config.roi))
                raisePowerWarning();
            return true;
        }
        return false;
    }

    void setRoi(const RoiRect& roi)
    {
        if (useRoi && roi.isValid()) {
            g.ax1 = qRound(roi.left * double(c.w));
            g.ay1 = qRound(roi.top * double(c.h));
            g.ax2 = qRound(roi.right * double(c.w));
            g.ay2 = qRound(roi.bottom * double(c.h));
        }
        else {
            g.ax1 = 0;
            g.ay1 = 0;
            g.ax2 = c.w;
            g.ay2 = c.h;
        }
        r.x1 = g.ax1;
        r.y1 = g.ay1;
        r.x2 = g.ax2;
        r.y2 = g.ay2;
    }

    void setRois(const QList<RoiRect>& rois)
    {
        bool powerWarning = false;
        if (_config.rois.size() != rois.size())
            powerWarning = true;
        else {
            for (int i = 0; i < rois.size(); i++)
                if (!_config.rois.at(i).isEqual(rois.at(i))) {
                    powerWarning = true;
                    break;
                }
        }
        _config.rois = rois;
        for (auto roi : std::as_const(_config.rois)) roi.fix();
        saveConfig(0);
        if (powerWarning)
            raisePowerWarning();
    }

    void setRois(const QList<QPointF>& points)
    {
        QList<RoiRect> rois;
        for (const auto& p : points) {
            RoiRect roi;
            roi.left = p.x() - _config.mroiSize.w / 2.0;
            roi.top = p.y() - _config.mroiSize.h / 2.0;
            roi.right = p.x() + _config.mroiSize.w / 2.0;
            roi.bottom = p.y() + _config.mroiSize.h / 2.0;
            rois << roi;
        }
        setRois(rois);
    }

    void setRoisSize(const FrameSize& sz)
    {
        _config.mroiSize = sz;

        QList<QPointF> points;
        for (const auto& roi : std::as_const(_config.rois)) {
            points << QPointF(
                (roi.left + roi.right) / 2.0,
                (roi.top + roi.bottom) / 2.0
            );
        }
        setRois(points);
    }

    void setRoiMode(RoiMode mode)
    {
        if (_config.roiMode != mode) {
            _config.roiMode = mode;
            saveConfig(0);
            raisePowerWarning();
        }
    }

    void setColorMap(const QString& colorMap)
    {
        if (_config.plot.colorMap != colorMap) {
            _config.plot.colorMap = colorMap;
            saveConfig(0);
        }
    }

    bool isRoiValid() const
    {
        return _config.roi.isValid();
    }

    PixelScale pixelScale() const
    {
        if (!_config.plot.rescale)
            return {};
        if (_config.plot.customScale.on)
            return _config.plot.customScale;
        return camera->sensorScale();
    }

    QString formatBrightness(double v) const
    {
        if (_config.plot.normalize)
            return QString::number(v, 'f', 3);
        return QString::number(v, 'f', 0);
    }
    

    TableRowsSpec tableRows() const
    {
        TableRowsSpec rows;
        rows.showSdev = _config.mavg.on;
        if (_config.roiMode == ROI_NONE || _config.roiMode == ROI_SINGLE) {
            rows.results << qApp->tr("Centroid");
        }
        else {
            for (int i = 0; i < _config.rois.size(); i++) {
                const auto& roi = _config.rois.at(i);
                if (roi.label.isEmpty())
                    rows.results << qApp->tr("Result #%1").arg(i + 1);
                else rows.results << roi.label;
            }
        }
        rows.aux
            << qMakePair(ROW_RENDER_TIME, qApp->tr("Acq. time"))
            << qMakePair(ROW_CALC_TIME, qApp->tr("Calc time"))
            << qMakePair(ROW_FRAME_ERR, qApp->tr("Errors"))
            << qMakePair(ROW_FRAME_DROPPED, qApp->tr("Dropped"))
            << qMakePair(ROW_FRAME_UNDERRUN, qApp->tr("Underrun"))
            << qMakePair(ROW_FRAME_INCOMPLETE, qApp->tr("Incomplete"));
        /*if (_cfg->showBrightness)
            rows.aux << qMakePair(ROW_BRIGHTNESS, qApp->tr("Brightness"));*/
        if (_config.power.on)
            rows.aux << qMakePair(ROW_POWER, qApp->tr("Power"));


        return rows;
    }

    QList<QPair<int, QString>> measurCols() const
    {
        QList<QPair<int, QString>> cols;
        /*if (_cfg->saveBrightness)
            cols << qMakePair(COL_BRIGHTNESS, qApp->tr("Brightness"));*/
        if (_config.power.on)
            cols << qMakePair(COL_POWER, qApp->tr("Power"));
        return cols;
    }

    bool editRoisSize()
    {
        auto scale = pixelScale();
        double imgW = scale.pixelToUnit(camera->width());
        double imgH = scale.pixelToUnit(camera->height());
        auto edW = new QSpinBox; edW->setRange(10, imgW);// , qRound(imgW * _config.mroiSize.w));
        auto edH = new QSpinBox; edH->setRange(10, imgH); //(10, imgH, qRound(imgH * _config.mroiSize.h));
        edW->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        edH->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        QString suffix = scale.on ? scale.unit : QStringLiteral("px");
        auto w = LayoutV({
            qApp->tr("Several ROIs are building around\n"
                "crosshairs, they have the same size:"),
            SpaceV(),
            qApp->tr("Width"), LayoutH({ edW, suffix }),
            SpaceV(),
            qApp->tr("Height"), LayoutH({ edH, suffix }),
            }).setMargin(0).makeWidgetAuto();
        bool ok = Dialog(w)
            .withContentToButtonsSpacingFactor(3)
            .windowModal()
            .exec();
        if (ok) {
            setRoisSize({
                double(edW->value()) / imgW,
                double(edH->value()) / imgH
                });
        }
        return ok;
    }



protected:
    BeamConfig _config;

    virtual void initConfigMore(ConfigDlgOpts& opts) {}
    virtual void loadConfigMore(QSettings*) {}
    virtual void saveConfigMore(QSettings*) {}
};


#endif // CAMERA_WORKER
