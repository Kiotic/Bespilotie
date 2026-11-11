#include "uav.h"
#include <QDebug>


void SurveyUAV::takeoff()            { qDebug().noquote() << type() + ": vertical takeoff"; }
void SurveyUAV::land()               { qDebug().noquote() << type() + ": precision landing"; }
QString SurveyUAV::type() const      { return "Survey UAV"; }
QString SurveyUAV::mission_type() const { return "terrain mapping"; }

CargoUAV::CargoUAV(double payload_kg) : payload_kg_(payload_kg < 0 ? 0 : payload_kg) {}
void CargoUAV::takeoff() {
    qDebug().noquote() << type() + QString(": heavy-lift takeoff, payload=%1 kg").arg(payload_kg_);
}
void CargoUAV::land()                { qDebug().noquote() << type() + ": soft landing with cargo release"; }
QString CargoUAV::type() const       { return "Cargo UAV"; }
QString CargoUAV::mission_type() const { return "payload transportation"; }
double CargoUAV::payload() const     { return payload_kg_; }
void CargoUAV::set_payload(double kg) {
    if (kg < 0) {
        qDebug().noquote() << "Error: payload mass cannot be negative, value ignored";
        return;
    }
    payload_kg_ = kg;
}

void ReconnaissanceUAV::takeoff()    { qDebug().noquote() << type() + ": silent catapult launch"; }
void ReconnaissanceUAV::land()       { qDebug().noquote() << type() + ": parachute landing"; }
QString ReconnaissanceUAV::type() const { return "Reconnaissance UAV"; }
QString ReconnaissanceUAV::mission_type() const { return "observation and monitoring"; }


void execute_test_mission(UAV& vehicle)
{
    qDebug().noquote() << "--- Test mission:" << vehicle.type() << "---";
    vehicle.takeoff();
    qDebug().noquote() << vehicle.type() + ": " + vehicle.mission_type();
    vehicle.land();
    qDebug().noquote() << "Mission completed for" << vehicle.type();
}
