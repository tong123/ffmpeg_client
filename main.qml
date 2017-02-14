import QtQuick 2.3
import QtQuick.Window 2.2
import QtQuick.Controls 1.3

Window {
    id: main_page
    width: wifi_client.get_desktop_width()
    height: wifi_client.get_desktop_height()
    visible: true
    property bool b_show: true
    Rectangle {
        id: discover_list_rect
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: main_page.width
        height: main_page.height
        visible: b_show
        property int  n_discover_index: -1
        ListModel{
            id: discover_model
        }
        Component {
            id: discover_delegate
            Item {
                id: wrapper
                width: main_page.width
                height: main_page.height/10
                Rectangle {
                    anchors.fill: parent
                    color: wrapper.ListView.isCurrentItem? "blue":"white"
                }

                Row{
                    anchors.centerIn: parent
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 8
                    Text {
                        id: ip_mac_txt
                        text: name
                        color: wrapper.ListView.isCurrentItem? "white":"black"
                        font.pixelSize: 30
                    }

                }
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 1
                    color: "black"
                }

                MouseArea {
                    id: mousearea
                    anchors.fill: parent
                    onClicked: {
                        wrapper.ListView.view.currentIndex = index
                        discover_list_rect.n_discover_index = index
                    }

                }
            }

        }
//        ScrollView {
//            anchors.horizontalCenter: parent.horizontalCenter
//            width: main_page.width
//            height: main_page.height/2

            ListView {
                id: discover_list
                anchors.fill: parent
                delegate: discover_delegate
                model: discover_model
                interactive: false
                focus: true
                Component.onCompleted: {
                    discover_list.currentIndex = -1
                    discover_model.clear()
                }
                Connections {
                    target: wifi_client
                    onUpdate_discover_model: {
//                        console.log("hello update_discover_model")
                        update_discover_list_model()
                        discover_list.currentIndex = discover_list_rect.n_discover_index
                    }
                }
            }
//        }
    }
    Rectangle{
        id: search_btn
        visible: b_show
        anchors.bottom: parent.bottom
        anchors.bottomMargin: main_page.height/10+60
        anchors.horizontalCenter: parent.horizontalCenter
        width: main_page.width
        height: main_page.height/10
        border.width: 0.6
        radius: 6

        Text {
            id: search_txt
            anchors.centerIn: parent
            font.pixelSize: 30
            text: qsTr("查  找")
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                wifi_client.search_online_device( )
            }
            onPressed: {
                search_btn.color = "blue"
                search_txt.color = "white"
            }

            onReleased: {
                search_btn.color = "white"
                search_txt.color = "black"
            }
        }
    }

    Rectangle{
        id: link_btn
        visible: b_show
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 30
        anchors.horizontalCenter: parent.horizontalCenter
        width: main_page.width
        height: main_page.height/10
        border.width: 0.6
        radius: 6
        enabled: discover_list.currentIndex != -1
        property bool b_unlinked_status: true
        color: discover_list.currentIndex != -1 ? "white":"lightgray"
        Text {
            id: link_txt
            anchors.centerIn: parent
            font.pixelSize: 30
            text: qsTr("连  接")
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
//                image_display.visible = true
                if( link_btn.b_unlinked_status ) {
                    wifi_client.connect_to_current_online_device( discover_list.currentIndex, true )
//                    link_btn.b_unlinked_status = false
//                    link_txt.text = qsTr("断  开")
//                    image_display.visible = true
                } else {
                    wifi_client.disconnect_current_server( )
                    link_btn.b_unlinked_status = true
                    link_txt.text = qsTr("连  接")
                }

            }
            onPressed: {
                link_btn.color = "blue"
                link_txt.color = "white"
            }

            onReleased: {
                link_btn.color = "white"
                link_txt.color = "black"
            }
        }
    }

    ImageDisplay {
        id: image_display
        anchors.fill: parent
//        visible: true
        visible: !b_show
    }

    function update_discover_list_model() {
        if( !image_display.visible ) {
            discover_model.clear()
            for( var i=0; i<wifi_client.get_mac_ip_list_count(); i++ ) {
                discover_model.append( {"name":wifi_client.get_current_mac_ip_content(i) } )
            }
        }
    }
    Connections {
        target: wifi_client
        onConnect_to_server_result: {
            image_display.visible = b_status
            if( b_status ) {
                link_btn.b_unlinked_status = false
                link_txt.text = qsTr("断  开")
            }
        }
    }
    Connections{
        target: wifi_client
        onDisconnect_net_status: {
            link_btn.b_unlinked_status = true
            link_txt.text = qsTr("连  接")
            image_display.visible = false
            main_page.visible = true
        }
    }
}
