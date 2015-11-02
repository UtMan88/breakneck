#ifndef ELEVATOR_BANK_PANEL_H
#define ELEVATOR_BANK_PANEL_H

class ElevatorScheduler;

class ElevatorBankPanel {
    const int mFloorNum;
    ElevatorScheduler* const mElevatorScheduler;

  public:
    enum class Direction { Up, Down };

    ElevatorBankPanel(int floorNum, ElevatorScheduler*);

    bool pushButton(const Direction& dir);
};
#endif // ELEVATOR_BANK_PANEL_H
