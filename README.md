This project implements multiprocess synchronization for following task (whole task see zadani_2024.pdf):

- We have 3 types of processes in the system: (0) main process, (1) ski bus and (2) skier.
- Every skier goes after breakfast to one boarding stop of the ski bus, where it is waiting for the bus to arrive.
- After the arrival of the bus to the stop skiers can board. If the capacity of the bus is full, the remaining skiers wait for the next bus arrival.
- Bus gradually serves all boarding stops and takes skiers to the exit stop at the cable car.
- If there are other skiers still waiting at bus stops, bus continues with next round.
