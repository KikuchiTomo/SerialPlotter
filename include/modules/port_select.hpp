#ifndef S_GTKMM_PORT_SELECT_HPP__
#define S_GTKMM_PORT_SELECT_HPP__

#include <gtkmm.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logger.hpp"
#include "serial_processer.hpp"
#include "port_select_presenter.hpp"

// TODO: 
// メンバにしたけど本来ならばサブクラス化して部品単位で切り出すべき.
// 今回は時間優先でめんどくさいので, このまま. このコードを動かす誰かがリファクタリングをすることを祈って.
class PortSelection: public Gtk::Window{
public:
 PortSelection() { 
  presenter_ = new PortSelectionPresenter();

  setupPortSelectButton();
  setupPortComboBox();
  setupRateComboBox();
  setupPortTreeView();
  setupRecordingView();
  setupPortArrangementBox();
  set_border_width(10);

  presenter_->setTreeModel(&tree_view_); 

  set_title("Serial Port Selection");

  // presenter_->view = this;
 }

 ~PortSelection() { 
  SNotice("discarded PortSelectionView");

  delete presenter_;
 }

private:
  void setupPortSelectButton(){
    select_button_ = Gtk::Button("Open");  
    select_button_.set_border_width(5);  
    select_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &PortSelection::viewDidClicked));       
  }

  void setupPortComboBox(){
      std::vector<std::string> ports;
      SerialProcesser::shared().getPorts(ports);
      for (const auto& e: ports) {
          select_port_.append(e.c_str());
      }
         
      select_port_.set_border_width(5);
      select_port_.set_size_request(380, 40);
  }

   void setupRateComboBox(){
      std::vector<std::string> rate = {"9600", "115200"};    
      for (const auto& e: rate) {
          select_rate_.append(e.c_str());
      }
         
      select_rate_.set_border_width(5);
      select_rate_.set_size_request(80, 40);
  }

  void setupPortArrangementBox() {
      serial_select_hbox_.pack_start(select_port_);
      serial_select_hbox_.pack_start(select_rate_);
      serial_select_hbox_.pack_start(select_button_);
      serial_select_vbox_.pack_start(serial_select_hbox_);
      serial_select_vbox_.pack_start(scroll_view_);
      serial_select_vbox_.pack_start(recording_hbox_);
      serial_select_vbox_.pack_start(state_label_);
      add(serial_select_vbox_);
      show_all_children();
      resize(600, 40);
  }

  void setupPortTreeView() {    
    scroll_view_.set_size_request(590, 300);
    scroll_view_.set_border_width(5);
    scroll_view_.add( tree_view_ );  
  }

  void updateStateView(const char* state){
    state_label_.set_text(state);    
  }

  void updateTimeCodeView(const char* time){
    time_label_.set_text(time);    
  }

  void setupRecordingView(){
    record_button_ = Gtk::Button("Record/Stop");  
    record_button_.set_border_width(5);
    record_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &PortSelection::viewDidRecordClicked));  
    // Time code label    
    time_label_.set_text("00:00:00.00");  
    auto time_context = time_label_.get_pango_context();  
    auto time_font = time_context->get_font_description();
    time_font.set_family("Monospace");
    time_font.set_weight(Pango::WEIGHT_ULTRABOLD);
    time_font.set_size(22 * PANGO_SCALE);  
    time_context->set_font_description(time_font);
    time_label_.set_halign(Gtk::ALIGN_CENTER);

    time_label_.set_size_request(380, 40);

    state_label_.set_text("STAND BY");    
    auto state_context = state_label_.get_pango_context();  
    auto state_font = state_context->get_font_description();
    state_font.set_family("Monospace");
    state_font.set_weight(Pango::WEIGHT_ULTRABOLD);
    state_font.set_size(22 * PANGO_SCALE);  
    state_context->set_font_description(time_font);
    state_label_.set_halign(Gtk::ALIGN_CENTER);


    // state_label_.set_border_width(5);
    recording_hbox_.pack_start(time_label_);
    recording_hbox_.pack_start(record_button_);
  }
 protected:
  void viewDidClicked() {
      SNotice("%s:%d ", select_port_.get_active_text().c_str(), stoi(std::string(select_rate_.get_active_text())));
      presenter_->viewDidClickOpenButton(
          select_port_.get_active_text().c_str(),
          stoi(std::string(select_rate_.get_active_text())));
  }

  void viewDidRecordClicked() { presenter_->viewDidClickRecordButton(); }
  PortSelectionPresenter* presenter_;

  Gtk::ScrolledWindow scroll_view_;
  Gtk::TreeView tree_view_;
  Gtk::Button select_button_;
  Gtk::ComboBoxText select_port_;
  Gtk::ComboBoxText select_rate_;
  Gtk::HBox serial_select_hbox_;
  Gtk::VBox serial_select_vbox_;
  Gtk::HBox recording_hbox_;
  Gtk::Label state_label_;
  Gtk::Label time_label_;
  Gtk::Button record_button_;
};

#endif