#ifndef MEASURE_SAVER_H
#define MEASURE_SAVER_H

#include <QDateTime>
#include <QEvent>
#include <QMap>
#include <QObject>
#include <QSharedPointer>
#include <QSettings>

#include <optional>

#include "CameraTypes.h"
#include "acquisitionsystem.h"

class CameraWorker;


struct MeasureConfig
{
    QString fileName;
    bool allFrames;
    int intervalSecs;
    bool average;
    bool durationInf;
    QString duration;
    bool saveImg;
    QString imgInterval;
    QSettings* s;
    void load(QSettings *s);
    void save(QSettings *s, bool min=false) const;
    void loadRecent();
    void saveRecent() const;
    int durationSecs() const;
    int imgIntervalSecs() const;
};



#define EPS(dx, dy) (qMin(dx, dy) / qMax(dx, dy))


class MeasureSaver : public QObject
{
    Q_OBJECT

public:
    static std::optional<MeasureConfig> configure();

    MeasureSaver();
    ~MeasureSaver();

    const MeasureConfig& config() const { return _config; }

    QString start(const MeasureConfig &cfg, AcquisitionSystem* cam);

    void setCaptureStart(const QDateTime &t) { _captureStart = t; }

signals:
    void finished();
    void interrupted(const QString &error);

protected:
    bool event(QEvent *event) override;

private:
    QDateTime _captureStart;
    QSharedPointer<QThread> _thread;
    MeasureConfig _config;
    QString _cfgFile, _imgDir;
    QMap<qint64, QString> _errors;
    int _width, _height, _bpp;
    double _scale = 1;
    int _duration = 0;
    qint64 _measureStart;
    qint64 _intervalBeg;
    qint64 _intervalLen;
    int _intervalIdx;
    double _avg_xc, _avg_yc, _avg_dx, _avg_dy, _avg_phi;
    double _avg_cnt;
    QMap<int, double> _multires_avg;
    QMap<int, int> _multires_avg_cnt;
    int _multires_cnt = 0;
    int _savedImgCount = 0;
    QList<int> _auxCols;
    QMap<int, double> _auxAvgVals;
    double _auxAvgCnt;


    void processMeasure(MeasureEvent *e);
    void saveImage(ImageEvent *e);

    template <typename T>
    QString formatTime(qint64 time, T fmt) {
        return _captureStart.addMSecs(time).toString(fmt);
    }
};

#endif // MEASURE_SAVER_H
