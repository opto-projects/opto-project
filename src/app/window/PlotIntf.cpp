#include "PlotIntf.h"

PlotIntf::PlotIntf(QGraphicsItem* parentitem, BeamInfoText* beamInfo, RectOverlay* roi)
    : _colorMap(""), _colorScale("")
{
    _parentitem = parentitem;
    _beamInfo = beamInfo;
    this->_roi = roi;
}

void PlotIntf::createOverlays()
{

}
 
void PlotIntf::initGraph(int w, int h)
{
    _w = w;
    _h = h;
    /*auto d = _colorMap->data();
    if (d->keySize() != w or d->valueSize() != h) {
        _beamData = new BeamColorMapData(w, h);
        _colorMap->setData(_beamData);
    } else {
        _beamData = static_cast<BeamColorMapData*>(d);
    }*/
}

uchar* PlotIntf::rawGraph() const
{
    return _beamData->bits();
}

void PlotIntf::invalidateGraph() const
{

}

void PlotIntf::cleanResult()
{
    _results.clear();
    _min = 0;
    _max = 0;
}

void PlotIntf::setResult(const QVector<CgnBeamResult>& r, double min, double max)
{
    _results = r;
    _min = min;
    _max = max;

}

template <class T> void trimList( QList<T>& list, int n)
{
    while (list.size() > n)
        delete list.takeLast();
}


void PlotIntf::showResult()
{
    const int resultCount = _results.size();
    for (int i = 0; i < resultCount; i++) {

        const CgnBeamResult& _res = _results.at(i);

        const double red = qDegreesToRadians(_res.phi);
        const double xc = _scale.pixelToUnit(_res.xc);
        const double yc = _scale.pixelToUnit(_res.yc);
        const double dx = _scale.pixelToUnit(_res.dx);
        const double dy = _scale.pixelToUnit(_res.dy);


        if (i >= _beamShapes.size()) {
            _beamShapes.append(new EllipseOverlay(_parentitem));
            auto beam = _beamShapes.at(i);
            beam->setPenColor(Qt::white);
        }

        auto beam = _beamShapes.at(i);
        beam->setValue(xc, yc, dx, dy, _res.phi);
    }

    if (_beamShapes.size() > resultCount) {
        trimList(_beamShapes, resultCount);
    }

    if (_beamInfo->isVisible() && resultCount >= 1 && !_results.at(0).nan)
    {
        const CgnBeamResult& r = _results.at(0);
        double eps = qMin(r.dx, r.dy) / qMax(r.dx, r.dy);
        _beamInfo->setText(QStringLiteral("Xc = %1\nYc = %2\nDx = %3\nDy = %4\nPhi = %5°\nE = %6")
            .arg(_scale.format(r.xc),
                _scale.format(r.yc),
                _scale.format(r.dx),
                _scale.format(r.dy))
            .arg(r.phi, 0, 'f', 1)
            .arg(eps, 0, 'f', 3));
    }
    else _beamInfo->setText({});


    /*_colorScale->setDataRange(QCPRange(_min, _max));
    if (_w > 0) _beamData->setKeyRange(QCPRange(0, _scale.pixelToUnit(_w)));
    if (_h > 0) _beamData->setValueRange(QCPRange(0, _scale.pixelToUnit(_h)));*/
}

template <class T> void toggleVisiblity(QList<T>& list, bool on)
{
    for (auto& item : list)
        item->setVisible(on);
}

void PlotIntf::setRawView(bool on)
{
    toggleVisiblity(_beamShapes, !on);
}