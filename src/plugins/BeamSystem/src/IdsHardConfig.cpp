#include "IdsHardConfig.h"

#include "optodevkit/CameraTypes.h"
#include "optodevkit/qmmcore.h"

#include "optodevkit/util/FloatingPoint.h"
#include "optodevkit/util/OriLayouts.h"
#include "optodevkit/util/OriDialogs.h"
#include "optodevkit/util/OriValueEdit.h"

#include <QApplication>
#include <QBoxLayout>
#include <QCheckBox>
#include <QClipboard>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QStyleHints>
#include <QWheelEvent>
#include <QVBoxLayout>
#include <QAction>
#include <QTextStream>
#include <QWidget> 
#include <QDebug> 

#define LOG_ID "HardConfig:"

// Exposure preset field names
#define PKEY_NAME "name"
#define PKEY_EXP "exposure"
#define PKEY_AEXP "auto_exposure"
#define PKEY_FPS "frame_rate"
#define PKEY_AGAIN "analog_gain"
#define PKEY_DGAIN "digital_gain"


bool theSame(const AnyRecord &p1, const AnyRecord &p2)
{
    for (auto it = p1.constBegin(); it != p1.constEnd(); it++) {
        if (it.key() == QStringLiteral("name"))
            continue;
        bool ok;
        auto v1 = it.value().toDouble(&ok);
        if (!ok)
            return false;
        auto v2 = p2.value(it.key()).toDouble(&ok);
        if (!ok)
            return false;
        // Humans should see the same text, if they don't then values are different
        if (QString::number(v1) != QString::number(v2)) {
            //qDebug() << "not same numbers" << v1 << v2 << QString::number(v1) << QString::number(v2);
            return false;
        }
    }
    return true;
}


#define PROP_CONTROL(Prop, title) { \
    auto label = new QLabel; \
    label->setWordWrap(true); \
    auto edit = new CamPropEdit; \
    edit->scrolled = [this](bool wheel, bool inc, bool big){ set##Prop##Fast(wheel, inc, big); }; \
    edit->connect(edit, &ValueEdit::focused, edit, [this](bool focus){ if (!focus) show##Prop(); }); \
    edit->connect(edit, &ValueEdit::keyPressed, edit, [this](int key){ \
        if (key == Qt::Key_Return || key == Qt::Key_Enter) set##Prop(); }); \
    auto btn = new QPushButton(tr("Set")); \
    btn->setFixedWidth(50); \
    btn->connect(btn, &QPushButton::pressed, btn, [this]{ set##Prop(); }); \
    auto group = LayoutV({label, LayoutH({edit, btn})}).makeGroupBox(title); \
    groups << group; \
    layout->addWidget(group); \
    lab##Prop = label; \
    ed##Prop = edit; \
    group##Prop = group; \
}

#define CHECK_PROP_STATUS(Prop, getStatus) \
    if (getStatus()) { \
        lab##Prop->setText(tr("Not configurable")); \
        ed##Prop->setDisabled(true); \
    } else show##Prop();

#define PROP(Prop, setProp, getProp, getRange) \
    QLabel *lab##Prop; \
    CamPropEdit *ed##Prop; \
    QGroupBox *group##Prop; \
    \
    void show##Prop() { \
        auto edit = ed##Prop; \
        auto label = lab##Prop; \
        double value, min, max, step; \
        bool res = getProp(value); \
        if (!res) { \
            label->setText(QString::number(res)); \
            edit->setValue(0); \
            edit->setDisabled(true); \
            props[#Prop] = 0; \
            return; \
        } \
        if (ed##Prop == edFps && roundFps) { \
            /* Sometimes camera returns not exactly the same that was set
             * e.g. 14.9988 instead of 15, or 9.9984 instead of 10
             * We are not interested in such small differences
            */ \
            value = qRound(value * 100.0) / 100.0; \
        } else if (ed##Prop == edExp && roundExp) { \
            value = qRound(value / 10.0) * 10; \
        } \
        /*qDebug() << "showProp" << #Prop << value;*/ \
        edit->setValue(value); \
        edit->setDisabled(false); \
        props[#Prop] = value; \
        res = getRange( min, max, step); \
        if (!res) \
            label->setText(QString::number(res)); \
        else { \
            if (ed##Prop == edFps && roundFps) { \
                min = qRound(min * 100.0) / 100.0; \
                max = qRound(max * 100.0) / 100.0; \
                label->setText(QString("<b>Min = %1, Max = %2</b>").arg(min, 0, 'f', 2).arg(max, 0, 'f', 2)); \
            } else if (ed##Prop == edExp && roundExp) { \
                min = qRound(min / 10.0) * 10; \
                max = qRound(max / 10.0) * 10; \
                label->setText(QString("<b>Min = %1, Max = %2</b>").arg(min, 0, 'f', 0).arg(max, 0, 'f', 0)); \
            } else { \
                label->setText(QString("<b>Min = %1, Max = %2</b>").arg(min, 0, 'f', 2).arg(max, 0, 'f', 2)); \
            }\
            props[#Prop "Min"] = min; \
            props[#Prop "Max"] = max; \
            props[#Prop "Step"] = step; \
        }\
        if (ed##Prop == edExp) \
            showExpFreq(value); \
        if (!bulkSetProps) \
            highightPreset();\
    } \
    double get##Prop##Raw()  { \
        double value = 0; \
        auto res = getProp( value); \
        if (res) \
            qWarning() << LOG_ID << "getPropRaw" << #Prop << res; \
        return value; \
    } \
    void set##Prop() { \
        double oldValue = props[#Prop]; \
        double newValue = ed##Prop->value(); \
        set##Prop##Raw(newValue, true); \
        /*if (ed##Prop != edFps)*/ \
           /* exposureChanged();*/ \
    } \
    bool set##Prop##Raw(double v, bool showErr) { \
        auto res = setProp(v); \
        if (res) { \
            if (showErr) \
                ;/*emit error("error::setProp");*/ \
            else \
                qWarning() << LOG_ID << "setPropRaw" << #Prop << (res); \
            return false; \
        } \
        /*qDebug() << "setPropRaw" << #Prop << v;*/ \
        if (ed##Prop == edFps) { \
            showExp(); \
            showFps(); \
            if (fpsLock > 0) { \
                fpsLock = v; \
                showFpsLock(); \
            } \
        } else if (ed##Prop == edExp) { \
            showExp(); \
            showFps(); \
            if (fpsLock > 0) { \
                showFpsLock(); \
                setFpsRaw(fpsLock, false); \
            } \
        } else show##Prop(); \
        return true; \
    } \
    void set##Prop##Fast(bool wheel, bool inc, bool big) { \
        double change = wheel \
            ? (big ? propChangeWheelBig : propChangeWheelSm) \
            : (big ? propChangeArrowBig : propChangeArrowSm); \
        double step = props[#Prop "Step"]; \
        double val = props[#Prop]; \
        double newVal; \
        if (inc) { \
            double max = props[#Prop "Max"]; \
            if (val >= max) return; \
            newVal = val * change; \
            if (newVal - val < step) newVal = val + step; \
            newVal = qMin(max, newVal); \
        } else { \
            double min = props[#Prop "Min"]; \
            if (val <= min) return; \
            newVal = val / change; \
            if (val - newVal < step) newVal = val - step; \
            newVal = qMax(min, newVal); \
        } \
        auto res = setProp( newVal); \
        if ((!res)) \
            ;/*emit error(IDS.getPeakError(res));*/ \
        if (ed##Prop == edFps) { \
            showExp(); \
            showFps(); \
            if (fpsLock > 0) { \
                fpsLock = newVal; \
                showFpsLock(); \
            } \
        } else if (ed##Prop == edExp) { \
            showExp(); \
            showFps(); \
            if (fpsLock > 0) { \
                showFpsLock(); \
                setFpsRaw(fpsLock, false); \
            } \
        } else show##Prop(); \
        if (ed##Prop != edFps) \
            exposureChanged(); \
    }


class CamPropEdit : public ValueEdit
{

public:
    std::function<void(bool, bool, bool)> scrolled;
protected:
    void keyPressEvent(QKeyEvent *e) override {
        if (e->key() == Qt::Key_Up) {
            scrolled(false, true, e->modifiers().testFlag(Qt::ControlModifier));
            e->accept();
        } else if (e->key() == Qt::Key_Down) {
            scrolled(false, false, e->modifiers().testFlag(Qt::ControlModifier));
            e->accept();
        } else
            ValueEdit::keyPressEvent(e);
    }
    void wheelEvent(QWheelEvent *e) override {
        if (hasFocus()) {
            scrolled(true, e->angleDelta().y() > 0, e->modifiers().testFlag(Qt::ControlModifier));
            e->accept();
        } else
            ValueEdit::wheelEvent(e);
    }
};


//------------------------------------------------------------------------------
//                             IdsHardConfigPanelImpl
//------------------------------------------------------------------------------

class IdsHardConfigPanelImpl: public QWidget
{
public:
    IdsHardConfigPanelImpl(CMMCore* hCam) : QWidget(), hCam(hCam)
    {
        auto layout = new QVBoxLayout(this);

        groupPresets = LayoutV({}).makeGroupBox(tr("Presets"));
        layout->addWidget(groupPresets);

        PROP_CONTROL(Exp, tr("Exposure (us)"))

        labExpFreq = new QLabel;
        groupExp->layout()->addWidget(labExpFreq);
        {
            auto label = new QLabel(tr("Percent of dynamic range:"));
            edAutoExp = new CamPropEdit;
            connect(edAutoExp, &ValueEdit::keyPressed, edAutoExp, [this](int key){
                if (key == Qt::Key_Return || key == Qt::Key_Enter) autoExposure(); });
            butAutoExp = new QPushButton(tr("Find"));
            butAutoExp->setFixedWidth(50);
            connect(butAutoExp, &QPushButton::pressed, this, &IdsHardConfigPanelImpl::autoExposure);
            auto group = LayoutV({label, LayoutH({edAutoExp, butAutoExp})}).makeGroupBox(tr("Autoexposure"));
            groups << group;
            layout->addWidget(group);

            auto actnCopyLog = new QAction(tr("Copy Report"), this);
            connect(actnCopyLog, &QAction::triggered, this, &IdsHardConfigPanelImpl::copyAutoExpLog);
            butAutoExp->addAction(actnCopyLog);
            butAutoExp->setContextMenuPolicy(Qt::ActionsContextMenu);
        }

        PROP_CONTROL(Fps, tr("Frame rate"));

        butFpsLock = new QPushButton(tr("Lock frame rate"));
        butFpsLock->setCheckable(true);
        connect(butFpsLock, &QPushButton::clicked, this, &IdsHardConfigPanelImpl::toggleFpsLock);
        groupFps->layout()->addWidget(butFpsLock);

        PROP_CONTROL(AnalogGain, tr("Analog gain"))
        PROP_CONTROL(DigitalGain, tr("Digital gain"))


        vertMirror = new QCheckBox(tr("Mirror vertically"));

        bool flipx = hCam->hasProperty("Camera", "FlipX");

        if (flipx) {
            vertMirror->setChecked(true);
        } else {
            vertMirror->setChecked(false);
            vertMirrorIPL = true;
        }
        connect(vertMirror, &QCheckBox::toggled, this, &IdsHardConfigPanelImpl::toggleVertMirror);
        layout->addWidget(vertMirror);

        horzMirror = new QCheckBox(tr("Mirror horizontally"));
        int flipy = QString::fromStdString(hCam->getProperty("Camera", "FlipY")).toInt();
        if (flipy == 1) {
            horzMirror->setChecked(true);
        } else {
            horzMirror->setChecked(false);
            horzMirrorIPL = true;
        }
        connect(horzMirror, &QCheckBox::toggled, this, &IdsHardConfigPanelImpl::toggleHorzMirror);
        layout->addWidget(horzMirror);

        // TODO: mirroring doesn't work, while works in IDS Cockpit for the same camera
        vertMirror->setVisible(false);
        horzMirror->setVisible(false);

        layout->addStretch();
    }

    bool ExposureTime_GetRange(double& min, double& max,double& step)
    {
        if (hCam->hasProperty("Camera", "Exposure"))
        {
            if (hCam->hasPropertyLimits("Camera", "Exposure"))
            {
                min = hCam->getPropertyLowerLimit("Camera", "Exposure");
                max = hCam->getPropertyUpperLimit("Camera", "Exposure");
                step = 0.1;
                return true;
            }
        }
        
        return false;
    }
    bool GetExposure( double& v)
    {
        v = hCam->getExposure();
        return 1;
    }
    bool SetExposure( double v)
    {
        hCam->setExposure(v);
        return 1;
    }


    bool ExposureTime_GetAccessStatus()
    {
        if (hCam->hasProperty("Camera", "Exposure"))
        {
            return hCam->isPropertyReadOnly("Camera", "Exposure");
        }
        return false;
    }
    
    bool FrameRate_GetRange(double& min, double& max, double& step)
    {
        if (hCam->hasProperty("Camera", "Fps"))
        {
            if (hCam->hasPropertyLimits("Camera", "Fps"))
            {
                min = hCam->getPropertyLowerLimit("Camera", "Fps");
                max = hCam->getPropertyUpperLimit("Camera", "Fps");
                step = 0.1;
                return true;
            }
        }
        return false;
    }
    bool GetFrameRate(double& v)
    {
        if (hCam->hasProperty("Camera", "Fps"))
        {
            std::string value = hCam->getProperty("Camera", "Fps");
            v = QString::fromStdString(value).toDouble();
            return true;
        }
        return false;
    }
    bool SetFrameRate(double v)
    {
        if (hCam->hasProperty("Camera", "Fps") && (!hCam->isPropertyReadOnly("Camera", "Fps")))
        {
            hCam->setProperty("Camera", "Fps", v);
            return true;
        }
        return false;
    }
    bool FrameRate_GetAccessStatus()
    {
        if (hCam->hasProperty("Camera", "Fps"))
        {
            return hCam->isPropertyReadOnly("Camera", "Fps");
        }
        return false;
    }

    bool AnalogGain_GetRange(double& min, double& max, double& step)
    {
        if (hCam->hasProperty("Camera", "AnalogGain"))
        {
            if (hCam->hasPropertyLimits("Camera", "AnalogGain"))
            {
                min = hCam->getPropertyLowerLimit("Camera", "AnalogGain");
                max = hCam->getPropertyUpperLimit("Camera", "AnalogGain");
                step = 1;
                return true;
            }
        }
        return false;
    }
    bool GetAnalogGain(double& v)
    {
        if (hCam->hasProperty("Camera", "AnalogGain"))
        {
            std::string value = hCam->getProperty("Camera", "AnalogGain");
            v = QString::fromStdString(value).toDouble();
            return true;
        }
        return false;
    }
    bool SetAnalogGain(double v)
    {
        if (hCam->hasProperty("Camera", "AnalogGain") && (!hCam->isPropertyReadOnly("Camera", "AnalogGain")))
        {
            hCam->setProperty("Camera", "AnalogGain", v);
            return true;
        }
        return false;
    }
    bool AnalogGain_GetAccessStatus()
    {
        if (hCam->hasProperty("Camera", "AnalogGain"))
        {
            return hCam->isPropertyReadOnly("Camera", "AnalogGain");
        }
        return false;
    }

    bool DigitalGain_GetRange(double& min, double& max, double& step)
    {
        if (hCam->hasProperty("Camera", "DigitalGain"))
        {
            if (hCam->hasPropertyLimits("Camera", "DigitalGain"))
            {
                min = hCam->getPropertyLowerLimit("Camera", "DigitalGain");
                max = hCam->getPropertyUpperLimit("Camera", "DigitalGain");
                step = 1;
                return true;
            }
        }
        return false;
    }
    bool GetDigitalGain(double& v)
    {
        if (hCam->hasProperty("Camera", "DigitalGain"))
        {
            std::string value = hCam->getProperty("Camera", "DigitalGain");
            v = QString::fromStdString(value).toDouble();
            return true;
        }
        return false;
    }
    bool SetDigitalGain(double v)
    {
        if (hCam->hasProperty("Camera", "DigitalGain") && (!hCam->isPropertyReadOnly("Camera", "DigitalGain")))
        {
            hCam->setProperty("Camera", "DigitalGain", v);
            return true;
        }
        return false;
    }
    bool DigitalGain_GetAccessStatus()
    {
        if (hCam->hasProperty("Camera", "DigitalGain"))
        {
            return hCam->isPropertyReadOnly("Camera", "DigitalGain");
        }
        return false;
    }


    PROP(Exp, SetExposure, GetExposure, ExposureTime_GetRange)
    PROP(Fps, SetFrameRate, GetFrameRate, FrameRate_GetRange)
    PROP(AnalogGain, SetAnalogGain, GetAnalogGain, AnalogGain_GetRange)
    PROP(DigitalGain, SetDigitalGain, GetDigitalGain, DigitalGain_GetRange)

    void showInitialValues()
    {
        bulkSetProps = true;

        CHECK_PROP_STATUS(Exp, ExposureTime_GetAccessStatus)
        CHECK_PROP_STATUS(Fps, FrameRate_GetAccessStatus)
        CHECK_PROP_STATUS(AnalogGain, AnalogGain_GetAccessStatus)
        CHECK_PROP_STATUS(DigitalGain, DigitalGain_GetAccessStatus)

        auto level = getCamProp(IdsHardConfigPanel::AUTOEXP_LEVEL).toInt();
        if (level == 0) level = 80;
        edAutoExp->setValue(level);

        fpsLock = getCamProp(IdsHardConfigPanel::FPS_LOCK).toDouble();
        showFpsLock();

        bulkSetProps = false;
    }

    void showExpFreq(double exp)
    {
        double freq = 1e6 / exp;
        QString s;
        if (freq < 1000)
            s = QStringLiteral("Corresponds to <b>%1 Hz</b>").arg(freq, 0, 'f', 1);
        else
            s = QStringLiteral("Corresponds to <b>%1 kHz</b>").arg(freq/1000.0, 0, 'f', 2);
        labExpFreq->setText(s);
    }

    void autoExposure()
    {
        if (props["Exp"] == 0) return;

        auto level = edAutoExp->value();
        if (level <= 0) level = 1;
        else if (level > 100) level = 100;
        edAutoExp->setValue(level);

        setCamProp(IdsHardConfigPanel::AUTOEXP_LEVEL, level);

        autoExp = AutoExp();
        autoExp->mImpl =this;
        autoExp->targetLevel = level / 100.0;
        autoExp->subStepMax = qMax(1, getCamProp(IdsHardConfigPanel::AUTOEXP_FRAMES_AVG).toInt());
        autoExp->getLevel = [this]{ watingBrightness = true; requestBrightness(this); };
        autoExp->showExpFps = [this](double exp, double fps){
            props["Exp"] = exp;
            props["Fps"] = fps;
            edExp->setValue(exp);
            edFps->setValue(fps);
            if (fpsLock > 0)
                showFpsLock();
        };
        autoExp->finished = [this]{ stopAutoExp(); };

        for (auto group : groups)
            group->setDisabled(true);
        if (!autoExp->start())
            stopAutoExp();
    }

    void stopAutoExp()
    {
        watingBrightness = false;
        for (auto group : groups)
            group->setDisabled(false);
        double fps = props["Fps"];
        if (auto res = SetFrameRate( fps); (res)) {
            qWarning() << LOG_ID << "Failed to restore FPS after autoexposure" << (res);
        }
        showExp();
        showFps();
        exposureChanged();
        if (fpsLock > 0) {
            showFpsLock();
            setFpsRaw(fpsLock, false);
        }
    }

    void copyAutoExpLog()
    {
        if (autoExp)
            qApp->clipboard()->setText(autoExp->logLines.join('\n'));
    }

    struct AutoExp
    {
        IdsHardConfigPanelImpl* mImpl;
        int step, subStep, subStepMax;
        double targetLevel, accLevel, fps;
        double exp, exp1, exp2, expMin, expMax, expStep;
        QStringList logLines;
        QString logLine;
        std::function<void()> getLevel;
        std::function<void()> finished;
        std::function<void(double, double)> showExpFps;

        class LogLine : public QTextStream
        {
        public:
            LogLine(QStringList &log, QString *line) : QTextStream(line), log(log), line(line) {}
            ~LogLine() { qDebug() << LOG_ID << "Autoexposure:" << (*line); log << (*line); }
            QStringList &log;
            QString *line;
        };

        LogLine log() {
            logLine = "";
            return LogLine(logLines, &logLine);
        }

        bool setExp(double v) {
            mImpl->SetExposure(v) ;
            double exp;
            mImpl->GetExposure(exp);
            showExpFps(exp, fps);
            return true;
        }

        bool start()
        {
            log() << "target_level=" << targetLevel << " avg_frames=" << subStepMax;

            if (auto res = mImpl->ExposureTime_GetRange(expMin, expMax, expStep); (res)) {
                QString msg = QString("Failed to get exposure range: %1").arg((res));
                error(msg);
                log() << msg;
                return false;
            }
            exp = expMin;
            if (auto res = mImpl->SetExposure( exp); (res)) {
                QString msg = QString("Failed to set exposure to %1: %2").arg(exp).arg((res));
                error(msg);
                log() << msg;
                return false;
            }
            double fpsMin, fpsMax, fpsInc;
            if (auto res = mImpl->FrameRate_GetRange( fpsMin, fpsMax, fpsInc); (res)) {
                QString msg = QString("Failed to get FPS range: %1").arg((res));
                error(msg);
                log() << msg;
                return false;
            }
            fps = fpsMax;
            if (auto res = mImpl->SetFrameRate( fps); (res)) {
                QString msg = QString("Failed to set FPS to %1: %2").arg(fps).arg((res));
                error(msg);
                log() << msg;
                return false;
            }
            exp1 = exp;
            exp2 = 0;
            step = 0;
            subStep = 0;
            accLevel = 0;
            getLevel();
            return true;
        }

        void doStep(double level)
        {
            accLevel += level;
            subStep++;

            log() << "step=" << step << " sub_step=" << subStep
                << " exp=" << exp << " fps=" << fps << " level=" << level
                << " avg_level=" << accLevel/double(subStep);

            if (subStep < subStepMax) {
                getLevel();
                return;
            }

            level = accLevel / double(subStep);

            if (qAbs(level - targetLevel) < 0.01) {
                log() << "stop_0=" << exp;
                goto stop;
            }

            if (level < targetLevel) {
                if (exp2 == 0) {
                    if (!setExp(qMin(exp1*2, expMax)))
                        goto stop;
                    // The above does not fail when setting higher-thah-max exposure
                    // It just clamps it to max and the loop never ends.
                    // So need an explicit check:
                    if (exp >= expMax) {
                        log() << "underexposed=" << exp;
                        
                        goto stop;
                    }
                    exp1 = exp;
                } else {
                    exp1 = exp;
                    if (!setExp((exp1+exp2)/2.0))
                        goto stop;
                    if (qAbs(exp1 - exp) <= expStep) {
                        log() << "stop_1 " << exp;
                        goto stop;
                    }
                }
            } else {
                if (exp2 == 0) {
                    if (exp == expMin) {
                        log() << "overexposed=" << exp;
                        
                        goto stop;
                    }
                    exp2 = exp1;
                    exp1 /= 2.0;
                    if (!setExp((exp1+exp2)/2.0))
                        goto stop;
                } else {
                    exp2 = exp;
                    if (!setExp((exp1+exp2)/2.0))
                        goto stop;
                    if (qAbs(exp2 - exp) <= expStep) {
                        log() << "stop_2=" << exp;
                        goto stop;
                    }
                }
            }
            step++;
            subStep = 0;
            accLevel = 0;
            getLevel();
            return;

        stop:
            finished();
        }
    };

    void applySettings(bool onChange)
    {
        /*auto &s = AppSettings::instance();
        propChangeWheelSm = 1 + double(s.propChangeWheelSm) / 100.0;
        propChangeWheelBig = 1 + double(s.propChangeWheelBig) / 100.0;
        propChangeArrowSm = 1 + double(s.propChangeArrowSm) / 100.0;
        propChangeArrowBig = 1 + double(s.propChangeArrowBig) / 100.0;

        if (roundFps != s.roundHardConfigFps) {
            roundFps = s.roundHardConfigFps;
            if (onChange) showFps();
        }
        if (roundExp != s.roundHardConfigExp) {
           roundExp = s.roundHardConfigExp;
            if (onChange) showExp();
        }*/
    }


    void toggleVertMirror(bool on)
    {
        if (hCam->hasProperty("Camera", "FlipX"))
        {
            if (!hCam->isPropertyReadOnly("Camera", "FlipX"))
            {
                std::string value = hCam->getProperty("Camera", "FlipX");
                
                if (value == "0" && on)
                {
                    hCam->setProperty("Camera", "FlipX", "1");
                }
                else if(value == "1" && !on)
                {
                    hCam->setProperty("Camera", "FlipX", "0");
                }

                
            }
        }
        
    }

    void toggleHorzMirror(bool on)
    {
        if (hCam->hasProperty("Camera", "FlipY"))
        {
            if (!hCam->isPropertyReadOnly("Camera", "FlipY"))
            {
                std::string value = hCam->getProperty("Camera", "FlipY");

                if (value == "0" && on)
                {
                    hCam->setProperty("Camera", "FlipY", "1");
                }
                else if (value == "1" && !on)
                {
                    hCam->setProperty("Camera", "FlipY", "0");
                }


            }
        }
    }

    inline AnyRecords* getPresets()
    {
        return getCamProp(IdsHardConfigPanel::EXP_PRESETS).value<AnyRecords*>();
    }

    inline AnyRecord makePreset()
    {
        return {
            { PKEY_EXP, props["Exp"] },
            { PKEY_AEXP, edAutoExp->value() },
            { PKEY_FPS, props["Fps"] },
            { PKEY_AGAIN, props["AnalogGain"] },
            { PKEY_DGAIN, props["DigitalGain"] },
        };
    }

    void saveNewPreset()
    {
        auto preset = makePreset();
       InputTextOptions opts;
        opts.label = formatPreset(preset) + tr("<p>Enter preset name:</p>");
        if (inputText(opts)) {
            preset[PKEY_NAME] = opts.value;
            auto presets = getPresets();
            presets->append(preset);
            setCamProp(IdsHardConfigPanel::EXP_PRESETS, {});
            QApplication::postEvent(this, new UpdateSettingsEvent());
        }
    }

    QString formatPreset(const AnyRecord &preset)
    {
        QStringList s;
        s << QStringLiteral("<code>");
        // Trailing spaces are right margin for better look in the info window
        s << tr("Exposure(us):  <b>%1</b>    <br>").arg(preset[PKEY_EXP].toDouble());
        s << tr("Autoexposure:  <b>%1%</b><br>").arg(preset[PKEY_AEXP].toInt());
        s << tr("Frame rate:    <b>%1</b><br>").arg(preset[PKEY_FPS].toDouble());
        s << tr("Analog gain:   <b>%1</b><br>").arg(preset[PKEY_AGAIN].toDouble());
        s << tr("Digital gain:  <b>%1</b><br>").arg(preset[PKEY_DGAIN].toDouble());
        s << QStringLiteral("</code>");
        QString r = s.join(QString());
        // QLabel collapses spaces even in code block
        r.replace(' ', QStringLiteral("&nbsp;"));
        return r;
    }

    void highightPreset()
    {
        if (!getCamProp)
            return;
        for (auto b : presetButtons)
            b->setChecked(false);
        auto props = makePreset();
        auto presets = getPresets();
        for (int i = 0; i < presets->size(); i++) {
            auto const &preset = presets->at(i);
            if (theSame(props, preset)) {
                if (i < presetButtons.size()) {
                    presetButtons.at(i)->setChecked(true);
                    return;
                }
            }
        }
    }

    void makePresetButtons()
    {
        auto presets = getPresets();
        QMap<QString, QPushButton*> buttons;
        for (auto b : presetButtons) {
            groupPresets->layout()->removeWidget(b);
        }
        qDeleteAll(presetButtons);
        presetButtons.clear();
        for (int i = 0; i < presets->size(); i++) {
            const auto &preset = presets->at(i);

            QList<QAction*> actions;

            auto b = new QPushButton(preset["name"].toString());
            b->setCheckable(true);
            b->setContextMenuPolicy(Qt::ActionsContextMenu);
            connect(b, &QPushButton::clicked, this, &IdsHardConfigPanelImpl::applyPreset);
            groupPresets->layout()->addWidget(b);
            presetButtons << b;

            auto actInfo = new QAction(QIcon(":/toolbar/info"), tr("Preset info..."), b);
            connect(actInfo, &QAction::triggered, this, &IdsHardConfigPanelImpl::showPresetInfo);
            actions << actInfo;

            auto actRename = new QAction(tr("Rename preset..."), b);
            connect(actRename, &QAction::triggered, this, &IdsHardConfigPanelImpl::renamePreset);
            actions << actRename;

            auto actUpdate = new QAction(tr("Save current values to preset..."), b);
            connect(actUpdate, &QAction::triggered, this, &IdsHardConfigPanelImpl::updatePreset);
            actions << actUpdate;

            auto actSep = new QAction(b);
            actSep->setSeparator(true);
            actions << actSep;

            auto actDel = new QAction(QIcon(":/toolbar/trash"), tr("Remove preset..."), b);
            connect(actDel, &QAction::triggered, this, &IdsHardConfigPanelImpl::removePreset);
            actions << actDel;

            for (auto a : actions) a->setData(i);
            b->addActions(actions);
        }
        auto b = new QPushButton(tr(" Add new preset..."));
        b->setIcon(QIcon(":/toolbar/plus"));
        connect(b, &QPushButton::clicked, this, &IdsHardConfigPanelImpl::saveNewPreset);
        groupPresets->layout()->addWidget(b);
        presetButtons << b;
        adjustSize();
        highightPreset();
    }

    void remakePresetButtons()
    {
        QApplication::postEvent(this, new UpdateSettingsEvent());
    }

    void applyPreset()
    {
        PresetsHandler(this, presetButtons.indexOf(qobject_cast<QPushButton*>(sender()))).apply();
    }

    void showPresetInfo()
    {
        PresetsHandler(this, qobject_cast<QAction*>(sender())->data().toInt()).info();
    }

    void renamePreset()
    {
        PresetsHandler(this, qobject_cast<QAction*>(sender())->data().toInt()).rename();
    }

    void updatePreset()
    {
        PresetsHandler(this, qobject_cast<QAction*>(sender())->data().toInt()).update();
    }

    void removePreset()
    {
        PresetsHandler(this, qobject_cast<QAction*>(sender())->data().toInt()).remove();
    }

    struct PresetsHandler
    {
        AnyRecords *presets;
        AnyRecord *preset = nullptr;
        IdsHardConfigPanelImpl *parent;
        int index;

        PresetsHandler(IdsHardConfigPanelImpl *parent, int index) : parent(parent), index(index)
        {
            presets = parent->getPresets();
            if (index < 0 || index >= presets->size()) {
                qWarning() << "Preset index out of range:" << index << "| Presets count:" << presets->size();
                return;
            }
            preset = &((*presets)[index]);
        }

        void apply()
        {
            if (!preset) return;
            bool ok;
            parent->bulkSetProps = true;
            QStringList failed;
            if (auto v = (*preset)[PKEY_DGAIN].toDouble(&ok); ok) {
                if (!parent->setDigitalGainRaw(v, false)) {
                    failed << PKEY_DGAIN;
                    qDebug() << LOG_ID << "Preset not fully applied:" << PKEY_DGAIN
                        << "| preset =" << v << "| camera =" << parent->getDigitalGainRaw();
                }
            }
            if (auto v = (*preset)[PKEY_AGAIN].toDouble(&ok); ok) {
                if (!parent->setAnalogGainRaw(v, false)) {
                    failed << PKEY_AGAIN;
                    qDebug() << LOG_ID << "Preset not fully applied:" << PKEY_AGAIN
                        << "| preset =" << v << "| camera =" << parent->getAnalogGainRaw();
                }
            }
            if (auto v = (*preset)[PKEY_EXP].toDouble(&ok); ok) {
                if (!parent->setExpRaw(v, false)) {
                    failed << PKEY_EXP;
                    qDebug() << LOG_ID << "Preset not fully applied:" << PKEY_EXP
                        << "| preset =" << v << "| camera =" << parent->getExpRaw();
                }
            }
            if (auto v = (*preset)[PKEY_FPS].toDouble(&ok); ok) {
                if (!parent->setFpsRaw(v, false)) {
                    failed << PKEY_FPS;
                    qDebug() << LOG_ID << "Preset not fully applied:" << PKEY_FPS
                        << "| preset =" << v << "| camera =" << parent->getFpsRaw();
                }
            }
            if (auto v = (*preset)[PKEY_AEXP].toInt(&ok); ok) {
                parent->edAutoExp->setValue(v);
                parent->setCamProp(IdsHardConfigPanel::AUTOEXP_LEVEL, v);
            }
            parent->bulkSetProps = false;
            parent->highightPreset();
            parent->exposureChanged();
            if (!failed.empty())
                ;// Ori::Dlg::warning(tr("Preset are not fully applied. Failed to set some properties: %1").arg(failed.join(", ")));
        }

        void info()
        {
            if (!preset) return;
            //Ori::Dlg::info(parent->formatPreset(*preset));
        }

        void rename()
        {
            if (!preset) return;
            InputTextOptions opts;
            opts.value = (*preset)[PKEY_NAME].toString();
            opts.label = parent->formatPreset(*preset) + tr("<p>A new name for preset:</p>");
            if (inputText(opts)) {
                (*preset)[PKEY_NAME] = opts.value;
                parent->setCamProp(IdsHardConfigPanel::EXP_PRESETS, {});
                parent->presetButtons.at(index)->setText(opts.value);
            }
        }

        void update()
        {
            if (!preset) return;
            auto newPreset = parent->makePreset();
            auto confirm = parent->formatPreset(newPreset) +
                tr("<p>Update <b>%1</b> with these values?</p>").arg((*preset)[PKEY_NAME].toString());
            if (yes(confirm)) {
                (*presets)[index] = newPreset;
                parent->setCamProp(IdsHardConfigPanel::EXP_PRESETS, {});
                parent->highightPreset();
            }
        }

        void remove()
        {
            if (!preset) return;
            if (yes(tr("The preset will be removed:<br><br><b>%1</b>").arg((*preset)[PKEY_NAME].toString()))) {
                presets->removeAt(index);
                parent->setCamProp(IdsHardConfigPanel::EXP_PRESETS, {});
                parent->remakePresetButtons();
            }
        }
    };

    void showFpsLock()
    {
        bool locked = fpsLock > 0;
        bool lockOk = (int)qRound(fpsLock * 100) == (int)qRound(props["Fps"] * 100);
        butFpsLock->setChecked(locked);
        butFpsLock->setText(locked ? tr(" Target: %1 FPS").arg(fpsLock) : tr(" Lock frame rate"));
        butFpsLock->setIcon(QIcon(!locked ? ":/toolbar/lock_on" : (lockOk ? ":/toolbar/ok" : ":/toolbar/exclame")));
        if (locked && !lockOk)
            butFpsLock->setToolTip(tr("Target value is out of available range"));
        else butFpsLock->setToolTip({});
    }

    void toggleFpsLock(bool checked)
    {
        fpsLock = checked ? props["Fps"] : 0;
        setCamProp(IdsHardConfigPanel::FPS_LOCK, fpsLock);
        showFpsLock();
    }

    CMMCore* hCam ;
    QList<QGroupBox*> groups;
    CamPropEdit *edAutoExp;
    QPushButton *butAutoExp;
    QLabel *labExpFreq;
    QGroupBox *groupPresets;
    QList<QPushButton*> presetButtons;
    QCheckBox *vertMirror, *horzMirror;
    bool vertMirrorIPL = false, horzMirrorIPL = false;
    QMap<const char*, double> props;
    double propChangeWheelSm, propChangeWheelBig;
    double propChangeArrowSm, propChangeArrowBig;
    bool watingBrightness = false;
    bool closeRequested = false;
    bool bulkSetProps = false;
    bool roundFps = true;
    bool roundExp = true;
    double fpsLock = 0;
    QPushButton *butFpsLock;
    std::function<void(QObject*)> requestBrightness;
    std::function<QVariant(IdsHardConfigPanel::CamProp)> getCamProp;
    std::function<void(IdsHardConfigPanel::CamProp, QVariant)> setCamProp;
    std::function<void()> exposureChanged;
    std::optional<AutoExp> autoExp;

protected:
    void closeEvent(QCloseEvent *e) override
    {
        QWidget::closeEvent(e);
        // Event loop will crash if there is no event receiver anymore
        // Wait for the next brightness event and close after that
        if (watingBrightness) {
            closeRequested = true;
            e->ignore();
        }
    }

    bool event(QEvent *event) override
    {
        if (auto e = dynamic_cast<BrightEvent*>(event); e) {
            watingBrightness = false;
            if (closeRequested)
                close();
            else if (autoExp)
                autoExp->doStep(e->level);
            return true;
        }
        if (dynamic_cast<UpdateSettingsEvent*>(event)) {
            makePresetButtons();
            return true;
        }
        return QWidget::event(event);
    }
};

//------------------------------------------------------------------------------
//                              IdsHardConfigPanel
//-----------------------------------------------------------------------------

IdsHardConfigPanel::IdsHardConfigPanel(CMMCore* hCam  ,
    std::function<QVariant(CamProp)> getCamProp,
    std::function<void(CamProp, QVariant)> setCamProp,
    std::function<void(QObject*)> requestBrightness,
    std::function<void()> exposureChanged,
    QWidget *parent) : HardConfigPanel(parent)
{

    _impl = new IdsHardConfigPanelImpl(hCam);
    _impl->getCamProp = getCamProp;
    _impl->setCamProp = setCamProp;
    //_impl->exposureChanged = exposureChanged;
    _impl->requestBrightness = requestBrightness;
    _impl->applySettings(false);
    _impl->makePresetButtons();
    _impl->showInitialValues();
    _impl->highightPreset();

    auto scroll = new QScrollArea;
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(_impl);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(scroll);
}

void IdsHardConfigPanel::setReadOnly(bool on)
{
    for (auto group : _impl->groups)
        group->setDisabled(on);
}

bool IdsHardConfigPanel::event(QEvent *event)
{
    if (dynamic_cast<UpdateSettingsEvent*>(event)) {
        _impl->makePresetButtons();
        return true;
    }
    return QWidget::event(event);
}


