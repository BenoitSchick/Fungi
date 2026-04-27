/***************************************************************************//**
 *   @file   myError.h
 *   @brief  Personal error codes definition
 *   @author Andrea Enrile
********************************************************************************/

#ifndef MYERROR_H_
#define MYERROR_H_


#define SETUP_FAILED	 			 (240)	/* ADC setup failed */
#define PWRON_FAILED	 			 (241)	/* ADC power on failed */
#define REGISTER_WR_FAILED 			 (242)	/* A register write failed */
#define MSG_SENDER_CREATION_FAILED	 (243)	/* Message queue failed to create sender */
#define MSG_RECEIVER_CREATION_FAILED (244)	/* Message queue failed to create receiver */
#define DIR_CREATION_FAILED 		 (245)	/* Error in crating a directory */
#define FILE_OPEN_FAILED 			 (246)	/* Error in opening a file */
#define THREAD_CREATION_ERROR 		 (247)	/* Error in crating a thread */
#define MSG_SEND_FAILED	 		 	 (248)	/* Message queue failed to send */
#define MSG_RECEIVE_FAILED 		 	 (249)	/* Message queue failed to receive */
#define MSG_QUEUE_REMOVE_FAILED	 	 (250)	/* Message queue failed remove queue */

// Conversion thread errors
#define INVALID_POINTER 	(140)	/* Invalid pointer */
#define CONV_RDY_ERROR 		(141)	/* Error while waiting on convertion ready */
#define DATA_RD_ERROR 		(142)	/* Error while reading data */
#define NRDY_CONV_RDY_ERROR (143)	/* Not ready even after a conv_ready check */
#define ADC_ERROR_REG_FLAG  (144)	/* Error flag got activated, check printf msg to have details */
#define PWRON_FLAG_ERROR    (145)	/* ADC is not ready as power on flag is active */
#define WRONG_CHANNEL		(146)	/* Wrong channel is active */

#endif // MYERROR_H_
