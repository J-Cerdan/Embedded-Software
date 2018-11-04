/*! @file
 *
 *  @brief Used to manage threads
 *
 *  This contains definitions used to manage the threads for the RTOS
 *
 *  @author Amir Hussein & Joseph Cerdan
 *  @date 2018-10-12
 */
/*!
**  @addtogroup ThreadManage_module ThreadManage module documentation
**  @{
*/

/***************************Thread Priority**************************/
#define INIT_THREAD 0
#define CH00_THREAD 1
#define CH01_THREAD 2
#define UART_RX_THREAD 3
#define UART_TX_THREAD 4
#define PIT_THREAD 5
#define RTC_THREAD 6
#define FTM_THREAD 7
#define PACKET_THREAD 8

#define THREAD_STACK_SIZE 100

/*!
** @}
*/
