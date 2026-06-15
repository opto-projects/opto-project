#include "CameraWorker.h"

CameraWorker::CameraWorker()
{
    _configGroup = "CameraWorker";

    measurBuf1.resize(MEASURE_BUF_SIZE);
    measurBuf2.resize(MEASURE_BUF_SIZE);
    measurBufs[0] = measurBuf1.data();
    measurBufs[1] = measurBuf2.data();
    measurs = measurBufs[0];

    tableData = [this] {
        QMap<int, CamTableData> data = {
            { ROW_RENDER_TIME, {avgAcqTime} },
            { ROW_CALC_TIME, {avgCalcTime} },
            { ROW_FRAME_ERR, {framesErr, CamTableData::COUNT, framesErr > 0} },
            { ROW_FRAME_DROPPED, {framesDropped, CamTableData::COUNT, framesDropped > 0} },
            { ROW_FRAME_UNDERRUN, {framesUnderrun, CamTableData::COUNT, framesUnderrun > 0} },
            { ROW_FRAME_INCOMPLETE, {framesIncomplete, CamTableData::COUNT, framesIncomplete > 0} },
        };
        return data;
    };
}
CameraWorker::~CameraWorker() {}
