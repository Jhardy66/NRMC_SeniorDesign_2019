/*  NRMC Signal Board 2019
 * QEI.h
 *
 *  Created on: Feb 23, 2019
 *      Authors: Jordan Hardy,Charles Rawlins, Abe Caputi
 */

#ifndef QEI_H_
#define QEI_H_

int statehandler(int state,int pst,int a);
extern int click_cnt1;
extern int click_cnt2;
void init(void);
void init2(void);
int QEI_Init(void);
#endif /* QEI_H_ */
