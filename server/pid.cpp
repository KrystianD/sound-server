#include "pid.h"

void pidInit(PID* pid, float Kp, float Ki, float Kd, float imin, float imax, float min, float max)
{
	pid->Kp = Kp;
	pid->Ki = Ki;
	pid->Kd = Kd;
	pid->isum = 0;
	pid->imin = imin;
	pid->imax = imax;
	pid->min = min;
	pid->max = max;
	pid->lastError = 0;
}
float pidUpdate(PID* pid, float current, float setpoint, float dt)
{
	float error = current - setpoint;
	
	pid->isum += pid->Ki * error * dt;
	
	float curErr = error - pid->lastError;
	pid->lastError = error;
	
	if (pid->Ki > 0.0f)
	{
		if (pid->isum > pid->imax)
			pid->isum = pid->imax;
		else if (pid->isum < pid->imin)
			pid->isum = pid->imin;
	}
	
	float val = pid->Kp * error + pid->isum + pid->Kd / dt * curErr;
	if (val > pid->max)
		val = pid->max;
	else if (val < pid->min)
		val = pid->min;
	return val;
}
