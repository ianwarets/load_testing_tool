#include <windows.h>

void no_pacing(void(*)(), void *, DWORD);
void fixed_pacing(void(*)(), void *, DWORD);
void delayed_pacing(void(*)(), void *, DWORD);

/**
 * Функция циклического выполнения действия потока. 
 * @param first - action to run in cycle
 * @param second - parameter to be passed to action function
 * @param third - pacing function that will run the action inside itself
 * @param fourth - delay in seconds to make between action iterations
 */
void run_action(void(*)(), void *, void(*)(), DWORD);