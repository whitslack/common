#include "scheduler.tcc"

template class Scheduler<std::chrono::system_clock>;
template class Scheduler<std::chrono::steady_clock>;
