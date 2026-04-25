import QtQuick 2.15
import QtLocation 5.15
import QtPositioning 5.15

Rectangle {
    id: root
    color: "#e2e8f0"
    property var suppliers: []
    property real storeLat: 36.8065
    property real storeLon: 10.1815
    property bool fitToSuppliers: true

    Plugin {
        id: osmPlugin
        name: "osm"
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: osmPlugin
        center: QtPositioning.coordinate(root.storeLat, root.storeLon)
        zoomLevel: 6.8
        copyrightsVisible: true
        activeMapType: supportedMapTypes.length > 0 ? supportedMapTypes[0] : null

        MapQuickItem {
            coordinate: QtPositioning.coordinate(root.storeLat, root.storeLon)
            anchorPoint.x: 7
            anchorPoint.y: 7
            sourceItem: Rectangle {
                width: 14
                height: 14
                radius: 7
                color: "#111827"
                border.color: "white"
                border.width: 2
            }
        }

        Repeater {
            model: root.suppliers
            delegate: MapQuickItem {
                id: supplierPin
                coordinate: QtPositioning.coordinate(modelData.latitude, modelData.longitude)
                anchorPoint.x: 7
                anchorPoint.y: 7
                sourceItem: Rectangle {
                    id: marker
                    width: 14
                    height: 14
                    radius: 7
                    color: modelData.color ? modelData.color : "#ef4444"
                    border.color: "#1f2937"
                    border.width: 1

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: tip.visible = true
                        onExited: tip.visible = false
                    }
                }

                Rectangle {
                    id: tip
                    visible: false
                    x: 10
                    y: -6
                    z: 99
                    color: "#0f172a"
                    radius: 6
                    opacity: 0.95
                    border.color: "#334155"
                    border.width: 1
                    width: Math.min(340, tipText.implicitWidth + 16)
                    height: tipText.implicitHeight + 12

                    Text {
                        id: tipText
                        anchors.fill: parent
                        anchors.margins: 6
                        color: "#f8fafc"
                        text: modelData.tooltip ? modelData.tooltip : ""
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

        Component.onCompleted: {
            if (!root.fitToSuppliers || root.suppliers.length === 0)
                return

            var minLat = root.storeLat
            var maxLat = root.storeLat
            var minLon = root.storeLon
            var maxLon = root.storeLon
            for (var i = 0; i < root.suppliers.length; ++i) {
                var s = root.suppliers[i]
                minLat = Math.min(minLat, s.latitude)
                maxLat = Math.max(maxLat, s.latitude)
                minLon = Math.min(minLon, s.longitude)
                maxLon = Math.max(maxLon, s.longitude)
            }
            var centerLat = (minLat + maxLat) / 2.0
            var centerLon = (minLon + maxLon) / 2.0
            map.center = QtPositioning.coordinate(centerLat, centerLon)

            var spread = Math.max(maxLat - minLat, maxLon - minLon)
            if (spread < 0.2) map.zoomLevel = 9.5
            else if (spread < 0.5) map.zoomLevel = 8.5
            else if (spread < 1.0) map.zoomLevel = 7.8
            else if (spread < 2.0) map.zoomLevel = 7.0
            else map.zoomLevel = 6.4
        }
    }
}
