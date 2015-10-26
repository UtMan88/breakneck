#ifndef ELEVATOR_BANK_PANEL_HH
#define ELEVATOR_BANK_PANEL_HH

class ElevatorScheduler;

class ElevatorBankPanel {
    const int mFloorNum;
    ElevatorScheduler* const mElevatorScheduler;

  public:
    enum class Direction { Up, Down };

    ElevatorBankPanel(int floorNum, ElevatorScheduler*);

    bool pushButton(const Direction& dir);
};
#endif // ELEVATOR_BANK_PANEL_HH
