#ifndef IDS_HARD_CONFIG_H
#define IDS_HARD_CONFIG_H


#include "optodevkit/HardConfigPanel.h"
class CMMCore;
class IdsHardConfigPanel : public HardConfigPanel
{
public:
    enum CamProp { AUTOEXP_LEVEL, AUTOEXP_FRAMES_AVG, EXP_PRESETS, FPS_LOCK };

    IdsHardConfigPanel(CMMCore* hCam,
        std::function<QVariant(CamProp)> getCamProp,
        std::function<void(CamProp, QVariant)> setCamProp,
        std::function<void(QObject*)> requestBrightness,
        std::function<void()> exposureChanged,
        QWidget *parent);

    void setReadOnly(bool on) override;

protected:
    bool event(QEvent *event) override;

private:
    class IdsHardConfigPanelImpl *_impl;
};

#endif // IDS_HARD_CONFIG_H
