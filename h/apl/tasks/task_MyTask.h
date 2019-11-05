/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef APL_MY_TASK_H
#define	APL_MY_TASK_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include "_root/generic/os_Globals.h"

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

/*! User Parameter Declarations 
* ************************************************************************************** 
 * Description:
 * LED Blink Intervals are declared here using physical values in seconds
 * Pre-compiler macros in task_MyTask.c are used to calculated the respective
 * counter values to meet these settings.    
* **************************************************************************************/
    
#define DEFAULT_INNTERVAL   300e-3  // normal blink rate period in seconds
#define FAULT_INNTERVAL     50e-3   // fault  blink rate period in seconds

/*! User Object API Declarations 
* ************************************************************************************** 
 * Description:
 * Each task should provide a set of specific, public data structures and enums 
 * allowing other software modules to monitor and control functions of this task   
* **************************************************************************************/

// State Machine States
typedef enum {
    MYTASK_INITIALIZE,  // State machine is in initialization
    MYTASK_IDLE,        // State machine is in IDLE mode (waiting doing nothing)
    MYTASK_RUN          // State machine is in RUN mode (executing normal function)
}MY_TASK_STATE_e;       // List of availalbe State Machine States

// Global MY_TASK Object for remote monitoring & control
typedef struct {
    volatile bool enable;       // ENABLE turns the LED control on and off
    volatile bool led_state;    // This value publishes the most recent state of the LED (on/off)
    volatile bool ready;        // Flag indicating the initialization process is complete and the task is ready
    volatile uint16_t interval; // LED blink rate interval (read only)
    volatile MY_TASK_STATE_e state; // Most recent state of the MY_TASK state machine
} MY_TASK_OBJECT_t;             // Example task flashing the debugging LED

extern volatile MY_TASK_OBJECT_t my_task;   // Global MY-TASK object

// Public function prototypes to be added to the scheduler
// or to be called from other software modules

extern volatile uint16_t task_MyTask_Initialize(void);  // Initializes the MY_TASK object and the GPIO controlling the LED
extern volatile uint16_t task_MyTask_Start(void);       // Enables  MY_TASK
extern volatile uint16_t task_MyTask_Execute(void);     // Executes MY_TASK normal operation sequence
extern volatile uint16_t task_MyTask_Stop(void);        // Disables MY_TASK and frees all used resources (incl. GPIO)
    
#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* APL_MY_TASK_H */

