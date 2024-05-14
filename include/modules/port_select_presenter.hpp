#ifndef S_PORT_SELECT_PRESENTER_GPP__
#define S_PORT_SELECT_PRESENTER_GPP__

#include "port_select.hpp"
#include "logger.hpp"
#include "serial_processer.hpp"
#include <gtkmm.h>
#include <string>

class PortSelectionPresenter : public SerialProcesserDelegate {
  public:
    // TODO: 名前が汚．直す．
    Gtk::TreeModelColumn<int> col_num_;
    Gtk::TreeModelColumn<int> col_baud_rate_;
    Gtk::TreeModelColumn<Glib::ustring> col_name_;
    Gtk::TreeModelColumn<Glib::ustring> col_state_;
    Gtk::TreeModel::ColumnRecord record_;
    Glib::RefPtr<Gtk::ListStore> liststore_;

    bool isRecording = false;

    // PortSelection *view;
    PortSelectionPresenter() {
        SerialProcesser::shared().delegate = this;
        record_.add(col_num_);
        record_.add(col_name_);
        record_.add(col_baud_rate_);
        record_.add(col_state_);
        liststore_ = Gtk::ListStore::create(record_);        
    }

    void setTreeModel(Gtk::TreeView *treeview) {
        treeview->set_model(liststore_);
        // ビューに表示する列を指定
        treeview->append_column("State", col_state_);
        treeview->append_column("ID", col_num_);
        treeview->append_column("Port", col_name_);
        treeview->append_column("Baud Rate", col_baud_rate_);
    }

    ~PortSelectionPresenter() { SerialProcesser::shared().delegate = NULL; }

    void viewDidClickOpenButton(const char *port_name, int baud_rate) {
        SLog("clicked openButton");

        bool connection =
            SerialProcesser::shared().createConnection(port_name, baud_rate);
        if (connection) {
            Gtk::TreeModel::Row row;
            row = *(liststore_->append());
            row[col_num_] = SerialProcesser::shared().getConnectionCount();
            row[col_name_] = port_name;
            row[col_baud_rate_] = baud_rate;
            row[col_state_] = "Up";
        }
    }

    void viewDidClickRecordButton() {
        SLog("clicked recordButton");

        if(isRecording == false){
            isRecording = true;
            SerialProcesser::shared().startRecording();
        }else{
            SerialProcesser::shared().endRecording();
        }
    }

    void serialProcesserConnected(int id) override {
        // view->updateStateView("Connected");
    }

    void serialProcesserDisconnected(int id) override {
        // view->updateStateView("Disconnected");
    }

    void serialProcesserTimeout(int id) override {
       //  view->updateStateView("Timeout");
    }

    void serialProcesserFailToConnect(int id) override {
        // view->updateStateView("Fail");
    }
};

#endif