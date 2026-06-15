#ifndef TABLE_INTF_H
#define TABLE_INTF_H

#include "optodevkit/beam/beam_calc.h"
#include "optodevkit/CameraTypes.h"

class QTableWidget;
class QTableWidgetItem;

using ResultId = int;
using RowIndex = int;

class TableIntf
{
public:
    TableIntf(QTableWidget *table);

    void setRows(const TableRowsSpec &rows);
    void setScale(const PixelScale& scale) { _scale = scale; }
    void setResult(const QVector<CgnBeamResult>& val, const QVector<CgnBeamResult>& sdev, const QMap<ResultId, CamTableData>& data);
    void cleanResult();
    void showResult();
    bool resultInvalid() const;
    bool isPowerRow(QTableWidgetItem *item);

private:
    struct ResultRow
    {
        QTableWidgetItem *val, *sdev;
    };
    struct ResultRows
    {
        std::optional<ResultRow> xc, yc, dx, dy, phi, eps;
    };
    QTableWidget *_table;
    ResultId _powerResult = -1;
    QMap<ResultId, RowIndex> _camRows;
    QMap<ResultId, CamTableData> _camData;
    QVector<ResultRows> _resRows;
    QVector<CgnBeamResult> _resData;
    QVector<CgnBeamResult> _resSdev;
    PixelScale _scale;
    int _warnColor;
    bool _showSdev = false;

    QTableWidgetItem* makeHeader(RowIndex &row, const QString& title);
    ResultRow makeRow(RowIndex &row, const QString& title);
    void setTextInvald(const std::optional<TableIntf::ResultRow> &it);
};

#endif // TABLE_INTF_H
