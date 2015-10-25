#include <elevator.hh>
#include <elevator_bank_panel.hh>
#include <elevator_scheduler.hh>

ElevatorBankPanel::ElevatorBankPanel(int floorNum, ElevatorScheduler* const scheduler)
    : mFloorNum(floorNum), mElevatorScheduler(scheduler) {
    assert(scheduler && "ElevatorScheduler cannot be null");
}

bool ElevatorBankPanel::pushButton(const ElevatorBankPanel::Direction& dir) {
    typedef Elevator::Direction Dir;
    if (dir == ElevatorBankPanel::Direction::Up) {
        return mElevatorScheduler->requestElevator(mFloorNum, Dir::Up);
    }
    if (dir == ElevatorBankPanel::Direction::Down) {
        return mElevatorScheduler->requestElevator(mFloorNum, Dir::Down);
    }
    return false;
}
