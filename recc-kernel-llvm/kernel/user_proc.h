#ifndef USER_PROC_H_
#define USER_PROC_H_
/*
    Copyright 2016 Robert Elder Software Inc.
    
    Licensed under the Apache License, Version 2.0 (the "License"); you may not 
    use this file except in compliance with the License.  You may obtain a copy 
    of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software 
    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT 
    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the 
    License for the specific language governing permissions and limitations 
    under the License.
*/

#define PID_INIT 0
#define PID_USER_PROC_1 1
#define PID_CLOCK_COUNTER 2
#define PID_UART1_OUT_READY_NOTIFIER 3
#define PID_UART1_OUT_SERVER 4
#define PID_UART1_IN_READY_NOTIFIER 5
#define PID_UART1_IN_SERVER 6
#define PID_COMMAND_SERVER 7

void user_proc_1(void);
void clock_tick_counter(void);
void uart1_out_ready_notifier(void);
void uart1_out_server(void);
void uart1_in_ready_notifier(void);
void uart1_in_server(void);
void command_server(void);

#endif
