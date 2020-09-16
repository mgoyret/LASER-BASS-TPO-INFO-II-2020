/**
*	\file AP_MdeE.h
*	\brief Resumen del archivo
*	\details Descripcion detallada del archivo
*	\author marcosgoyret
*	\date 15-09-2020 17:00:19
*/

#ifndef INC_MDEE_H_
#define INC_MDEE_H_

#define	TRUE    1
#define	FALSE   0

//!< Definicion de Estados
#define	LEYENDO     0
#define	PROCESANDO  1

#define ESPERANDO   0
#define GRABANDO    1

//!< Declaracion de la Maquina de Estados
void maquina_grabador(void);
void maquina_principal(void);


#endif /* INC_MDEE_H_ */
