#include <stdio.h>
#include <gtkmm/application.h>
#include "serial_processer.hpp"
#include "logger.hpp"
#include "config.hpp"
#include "port_select.hpp"
#include "ring_vector.hpp"


int main(int argc, char *argv[]){
    SerialProcesser::init();
    SNotice("Launch application");
    auto app = Gtk::Application::create(argc, argv, SERIAL_BUNDLE_ID);
    PortSelection portSelection;
    return app->run(portSelection);
    // Serial::RingVector<int> *b = new Serial::RingVector(15, 0);  
}