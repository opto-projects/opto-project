#ifndef CAMERAEXTENSIONPARAMETERS_H
#define CAMERAEXTENSIONPARAMETERS_H

#include <QString>
#include <QtGlobal>
#include <QMetaType>
#include <QVariant>

#define CAMERA_SELECTION "selected_camera"
#define CAMERA_ROTATION_ANGLE "camera_rotation_angle"
#define CAMERA_SNAPSHOT_SAVE_PATH "snapshot_save_path"

struct CameraExtensionParameters {
	QString selectedCamera;
	qreal rotationAngle;
	QString snapShotSavePath;
};
Q_DECLARE_METATYPE(CameraExtensionParameters)


#endif // CAMERAEXTENSIONPARAMETERS_H


