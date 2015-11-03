#include <random>
#include "elevator_bank_panel.h"
#include "elevator_scheduler.h"

int main(int argc, char** argv) {
    // Order is important, vector must be destroyed before ElevatorScheduler!
    std::unique_ptr<ElevatorScheduler> scheduler(new ElevatorScheduler());
    typedef std::vector<ElevatorBankPanel> List;
    List panels;

    panels.reserve(32);
    {
        auto* pScheduler = scheduler.get();
        for (int num = 0; num < 32; ++num) {
            panels.emplace_back(num+1, pScheduler);
        }
    }

    std::random_device randSeed;
    std::default_random_engine gen(randSeed());
    std::uniform_int_distribution<List::size_type> dis(0, 31);

    for (;;) {
        auto floor = dis(gen);
        if (floor % 2 == 0) {
            panels.at(floor).pushButton(ElevatorBankPanel::Direction::Down);
        } else {
            panels.at(floor).pushButton(ElevatorBankPanel::Direction::Up);
        }
        // std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen) * 100));
    }

    return 0;
}
