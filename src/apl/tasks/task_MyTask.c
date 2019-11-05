/*
 * File:   task_MyTask.c
 * Author: M91406
 *
 * Created on October 10, 2019, 1:03 AM
 */


#include <xc.h>
#include "apl/tasks/task_MyTask.h"

/* PRIVATE DECLARATIONS  */

// pre-compiler macros are used to calculate the effective number
// of scheduler calls to meet the given timing 
#define DEFAULT_TICKS   ((uint16_t)(DEFAULT_INNTERVAL/TASK_MGR_TIME_STEP) - 1)
#define FAULT_TICKS     ((uint16_t)(FAULT_INNTERVAL/TASK_MGR_TIME_STEP) - 1)

volatile uint16_t tick_scaler = 0;
volatile uint16_t tick_scale_max = DEFAULT_TICKS;

volatile MY_TASK_OBJECT_t my_task;

/* *************************** PRIVATE FUNCTIONS *************************** */

/*!task_MyTask_UpdateTimebase
 * ***********************************************************************************************
 * Private function determining the divider of the task tick rate of the operating system
 * to the desired user task execution tick.
 * 
 * Example:
 * The OS is running on a 100 us time base. Every 100 us ONE task of the most recently
 * selected task queue is executed. Thus, the effective call rate for an individual task
 * within a task queue is n x 100 us.
 * 
 * This example user task should be executed every 300 ms. A counter is used counting the
 * task manager calls determining when the task should be executed to meet the desired period. 
 * The counter maximum therefore is dependent on the OS tick rate and the number of tasks in the
 * recent task queue. 
 *
 * As the OS tick rate is constant, the following function is only called at task initialization 
 * and as soon as a change of task queues has been detected, calculating the required counter
 * maximum.
 * 
 * *********************************************************************************************** */

volatile uint16_t task_MyTask_UpdateTimebase(void) {
// By counting the function calls, the scheduler time base tick 
// period can be scaled down to individual task execution period

    if (task_mgr.op_mode.value == OP_MODE_FAULT)
        my_task.interval = FAULT_TICKS;
    else
        my_task.interval = DEFAULT_TICKS;
    
    tick_scale_max = (uint16_t)(my_task.interval / task_mgr.task_queue_ubound);
    
    return(1);
}

/* *************************** PUBLIC FUNCTIONS *************************** */
/*!task_MyTask_Initialize
 * ***********************************************************************************************
 * This public function is initializing the GPIO required to drive the debugging LED,
 * determines the task tick counter maximum to make the task operate at the desired tick rate
 * and initializes the monitoring and control parameters in the my_task data structure
 * *********************************************************************************************** */

volatile uint16_t task_MyTask_Initialize(void) {
    
    volatile uint16_t fres = 1;
    
    // determine the counter values based on execution rate
    // defined by the main scheduler tick rate and task queue length
    fres &= task_MyTask_UpdateTimebase(); 
    
    // Initialize the DBGLED GPIO used to drive the LED
    DBGLED_INIT_OUTPUT;
    DBGLED_WR = LED_OFF;

    // Initialize the MY_TASK data structure
    my_task.interval = DEFAULT_TICKS;   // Set blink rate to DEFAULT
    my_task.enable = false;             // Disable the task
    my_task.led_state = DBGLED_RD;      // Capture the most recent LED status
    my_task.state = MYTASK_IDLE;        // Set the initial state of the state machine
    my_task.ready = true;               // Set the READY-bit 
    
    return(fres);
    
}

/*!task_MyTask_Start
 * ***********************************************************************************************
 * This public function enables the task.
 * When the task is in a disabled state, it's active but not performing any observable action.
 * When the task is enabled, if toggles the debugging LED with the desired period.
 * *********************************************************************************************** */

volatile uint16_t task_MyTask_Start(void) {
    
    volatile uint16_t fres = 1;
    
    DBGLED_WR = LED_ON;     // Turn on LED
    my_task.enable = true;  // Enable MY_TASK
    
    return(fres);
    
}


/*!task_MyTask_Execute
 * ***********************************************************************************************
 * This public function is the user task state machine.
 * The user task should have publicly accessible parameters, allowing other tasks to monitor its
 * current state and to interact. In this example task 'my_task' allows external modules to 
 * set/change the blink-rate of the LED as well as to enable/disable the task.
 * *********************************************************************************************** */

volatile uint16_t task_MyTask_Execute(void) {
    
    volatile uint16_t fres = 1;
    static SYSTEM_OPERATION_MODE_t pre_op_mode;
    
    // ============================================================================
    // Check if call tick rate has changed; Update counter threshold when needed

    if (pre_op_mode.value != task_mgr.op_mode.value)
    { // When the operating system has switched task queues, the call rate may be different
      // and the counter values need to be updated to keep the blink-rate constant
        fres &= task_MyTask_UpdateTimebase();
        pre_op_mode.value = task_mgr.op_mode.value;
    }
    
    // ============================================================================
    // Counting function calls until the next user task execution event
    
    if (tick_scaler++ < tick_scale_max) 
        return(fres);
    else
        tick_scaler = 0;

    // ============================================================================
    // This is the user function state machine 
    
    switch (my_task.state) {
        
        // MYTASK_INITIALIZE will cover the use case when no external 
        // software module has called task_MyTask_Initialize() yet.
        // In this case the initialization is enforced by the state machine
        
        case MYTASK_INITIALIZE:
            fres &= task_MyTask_Initialize();
            if (my_task.ready)
                my_task.state = MYTASK_IDLE;
            break;
            
        // MYTASK_IDLE will be the initial state. The task will remain 
        // in this state until it's actively enabled by an external software 
        // module

        case MYTASK_IDLE:
            DBGLED_WR = LED_ON;
            if (my_task.enable)
                my_task.state = MYTASK_RUN;
            break;
            
        // MYTASK_RUN performs the 'normal' function of the task.

        case MYTASK_RUN:
            DBGLED_TOGGLE;
            break;
            
        // In case any other state is set (e.g. FAULT), the task
        // is stopped and all resources are freed up. A new 
        // initialization will be required to recover the task.
            
        default:
            task_MyTask_Stop();
            break;
    }
    
    // Capture most recent LED status
    my_task.led_state = DBGLED_RD;
    
    // ===================================================================
    
    return(fres);
    
}

/*!task_MyTask_Stop
 * ***********************************************************************************************
 * This public function disables task and resets its state machine.
 * Every user task should offer a public function allowing external modules to disable and 
 * reset its state. Ideally, every resource used by this task to be reset to its default state,
 * so that either other task can use them or to ensure a restart attempt is clean.
 * *********************************************************************************************** */

volatile uint16_t task_MyTask_Stop(void) {
    
    volatile uint16_t fres = 1;

    DBGLED_INIT_INPUT;                  // Reset GPIO to default state
    
    my_task.enable = false;             // Disable task
    my_task.ready = false;              // Reset INIT flag
    my_task.state = MYTASK_INITIALIZE;  // Reset state machine
    
    return(fres);
    
}

// EOF
