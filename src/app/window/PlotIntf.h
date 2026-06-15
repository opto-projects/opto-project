#ifndef PLOT_INTF_H
#define PLOT_INTF_H

#include "optodevkit/beam/beam_calc.h"
#include "optodevkit/CameraTypes.h"
#include "ImageView/imagedisplay.h"

typedef TextOverlay BeamInfoText;
typedef EllipseOverlay BeamShape;
typedef LineOverlay BeamStraightLine;

class QGraphicsItem;

/**
 * Provides access to graph data for camera threads
 * without knowledge about specific graph implementation
 * and avoiding necessity to include whole widget into camera modules
 */
class PlotIntf
{
public:
    PlotIntf(QGraphicsItem * parentitem, BeamInfoText* beamInfo, RectOverlay* roi);
    void createOverlays();
    void setScale(const PixelScale& scale) { _scale = scale; }
    void setResult(const QVector<CgnBeamResult>& r, double min, double max);
    void showResult();
    void cleanResult();
    void setRawView(bool on);

    void initGraph(int w, int h);
    uchar* rawGraph() const;
    void invalidateGraph() const;

private:
    int _w = 0, _h = 0;
    double _min, _max;
    PixelScale _scale;
    QGraphicsItem* _parentitem;
    QVector<CgnBeamResult> _results;
    BeamInfoText *_beamInfo;
    QString _colorMap;
    QString _colorScale;
    QImage *_beamData;
    QList<BeamShape*> _beamShapes;
    RectOverlay* _roi;
};

#endif // PLOT_INTF_H
