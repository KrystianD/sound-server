#ifndef __PID_H__
#define __PID_H__

typedef struct _PID
{
	float Kp, Ki, Kd;
	float isum;
	float imin, imax, min, max;

	float lastError;
} PID;

void pidInit(PID* pid, float Kp, float Ki, float Kd, float imin, float imax, float min, float max);
static inline void pidSet(PID* pid, float Kp, float Ki, float Kd, float imin, float imax, float min, float max)
{
	pid->Kp = Kp;
	pid->Ki = Ki;
	pid->Kd = Kd;
	pid->imin = imin;
	pid->imax = imax;
	pid->min = min;
	pid->max = max;
}
float pidUpdate(PID* pid, float current, float setpoint, float dt);

#endif
