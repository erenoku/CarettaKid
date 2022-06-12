#include "motors.hpp"
#include "constants.hpp"
#include "debug.hpp"
Motors::Motors(Sonic *right, int M1F, int M1B, int M2F, int M2B, int M1S, int M2S) : pin_m1f(M1F), pin_m1b(M1B), pin_m2f(M2F), pin_m2b(M2B), pin_m1s(M1S), pin_m2s(M2S), right(right), is_stuck(false), stuck_time(0) {
	
}

void Motors::setup() {
	pinMode(pin_m1f, OUTPUT);
	pinMode(pin_m1b, OUTPUT);

	pinMode(pin_m2f, OUTPUT);
	pinMode(pin_m2b, OUTPUT);
	
	pinMode(pin_m1s, OUTPUT);
	pinMode(pin_m2s, OUTPUT);
}

void Motors::cruise() {
	DEBUG_PRINT("Cruising...\n");
	digitalWrite(pin_m1f, HIGH);
	digitalWrite(pin_m1b, LOW);
	digitalWrite(pin_m2f, HIGH);
	digitalWrite(pin_m2b, LOW);
	analogWrite(pin_m1s, CRUISE_SPEED1);
	analogWrite(pin_m2s, CRUISE_SPEED2); 
}

void Motors::turn_right(){
	DEBUG_PRINT("Turning Right...\n");
	digitalWrite(pin_m1f, HIGH);
	digitalWrite(pin_m1b, LOW);
	digitalWrite(pin_m2f, LOW);
	digitalWrite(pin_m2b, HIGH);
	analogWrite(pin_m1s, TURNING_SPEED);	
	analogWrite(pin_m2s, TURNING_SPEED);
}

void Motors::step_back(){
	DEBUG_PRINT("Stepping Back...\n");
	digitalWrite(pin_m1f, LOW);
	digitalWrite(pin_m1b, HIGH);
	digitalWrite(pin_m2f, LOW);
	digitalWrite(pin_m2b, HIGH);

	analogWrite(pin_m1s, CRUISE_SPEED1);
	analogWrite(pin_m2s, CRUISE_SPEED2);	
}

void Motors::follow_the_walls_mode() {
	MSM.continuous_schedule(MotorStateMachine::States::FollowTheWallsDecide);
}

void Motors::go_around_mode() {
	MSM.continuous_schedule(MotorStateMachine::States::GoAroundDecide);
}

//Force the robot to stop whatever it is doing with its motors and step away from that puck!
void Motors::force_back_right() {
	MSM.current = MotorStateMachine::States::RivalPuckStepBack;
	MSM.reset_timer();
	step();
}

Motors::MotorStateMachine::MotorStateMachine() : time(0), current(None), cont(None) {

}

void Motors::MotorStateMachine::continuous_schedule(States s) {
	cont = s;
}

void Motors::MotorStateMachine::reset_timer() {
	time = millis();
}

bool Motors::MotorStateMachine::poll_and_switch() {
	DEBUG_PRINT("POLLING\n");
	if (current == None) {
		current = cont;
		return true;
	}
	if (durations[current] == -1) return false;
	States old = current;
	unsigned long newtime = millis();
	DEBUG_PRINT("TIME_DELTA: ");
	DEBUG_PRINT(newtime - time);
	DEBUG_PRINT("\n");
	if (newtime - time >= durations[current]) {
		States old = current;
		current = next[current];
		if (current == None) {
			current = cont;
		}
		time = newtime;
		return old != current;
	} else {
		return false;
	}
}

void Motors::main_loop() {
	bool updated = MSM.poll_and_switch();
	while (updated) {
		updated = step();
	}
}

bool Motors::step() {
	using States=MotorStateMachine::States;
	
	States old = MSM.current;
	DEBUG_PRINT(MSM.labels[MSM.current]);
	DEBUG_PRINT("\n");
	switch (MSM.current) {
		case States::None:

		break;
		case States::GoAroundDecide: {
			double DistanceR = right->distance();
			DEBUG_PRINT("DistanceR: ");
			DEBUG_PRINT(DistanceR);
			DEBUG_PRINT("\n");

			if ((DistanceR >= SONIC_STUCK_THRESHHOLD)) {
				if (!is_stuck) {
					is_stuck = true;
					stuck_time = millis();
				}
			} else {
				is_stuck = false;
			}

			
			if ( (DistanceR < SONIC_WALL_THRESHHOLD) || (is_stuck && (millis() - stuck_time >= 3000)) ) {
				MSM.current = States::GoAroundStepBack;
			} else if(DistanceR > SONIC_WALL_THRESHHOLD){
				MSM.current = States::GoAroundCruise;
			} else {
				MSM.current = States::None;
			}
		}
		break;
		case States::FollowTheWallsCruise:
		case States::GoAroundCruise:
			cruise();	
		break;
		case States::RivalPuckStepBack:
		case States::FollowTheWallsStepBack:
		case States::GoAroundStepBack:
			step_back();
		break;
		case States::RivalPuckTurnRight:
		case States::FollowTheWallsTurnRight:
		case States::GoAroundTurnRight:
			turn_right();
		break;
		case States::FollowTheWallsDecide: {
			double DistanceR = right->distance();

			if ((DistanceR >= SONIC_STUCK_THRESHHOLD)) {
				if (!is_stuck) {
					is_stuck = true;
					stuck_time = millis();
				}
			} else {
				is_stuck = false;
			}
			if ( (DistanceR < SONIC_WALL_THRESHHOLD) || (is_stuck && (millis() - stuck_time >= 3000)) ) {
				MSM.current = States::FollowTheWallsStepBack;
			} else  if(DistanceR > SONIC_WALL_THRESHHOLD){
				MSM.current = States::FollowTheWallsCruise;
			} else {
				MSM.current = States::None;
			}
		}
		break;
	}
	DEBUG_PRINT("NEXTSTEP: ");
	DEBUG_PRINT(old != MSM.current);
	DEBUG_PRINT("\n");
	if (old != MSM.current) {

		MSM.reset_timer();
	}
	return old != MSM.current;
}


const int Motors::MotorStateMachine::durations[States::LENGTH] = {
	-1,					//None = 0,
	0,					//GoAroundDecide,
	100,				//GoAroundCruise,
	700,				//GoAroundStepBack,
	1000,				//GoAroundTurnRight,
	0,					//FollowTheWallsDecide,
	100,				//FollowTheWallsCruise,
	700,				//FollowTheWallsStepBack,
	1000,				//FollowTheWallsTurnRight,
	700,				//RivalPuckStepBack,
	1000,				//RivalPuckTurnRight,
};

const Motors::MotorStateMachine::States Motors::MotorStateMachine::next[States::LENGTH] = {
	States::None,							//None = 0,
	States::None,							//GoAroundDecide, -> (GoAroundCruise | GoAroundStepBack | None)
	States::None,							//GoAroundCruise,
	States::GoAroundTurnRight,				//GoAroundStepBack,
	States::None,							//GoAroundTurnRight,
	States::None,							//FollowTheWallsDecide, -> (FollowTheWallsCruise | FollowTheWallsStepBack)
	States::None,							//FollowTheWallsCruise,
	States::FollowTheWallsTurnRight,		//FollowTheWallsStepBack,
	States::None,							//FollowTheWallsTurnRight,
	States::RivalPuckTurnRight,				//RivalPuckStepBack,
	States::None							//RivalPuckTurnRight,
};
#ifdef DEBUG
const char *const Motors::MotorStateMachine::labels[States::LENGTH] = {
	"None",
	"GoAroundDecide",
	"GoAroundCruise",
	"GoAroundStepBack",
	"GoAroundTurnRight",
	"FollowTheWallsDecide",
	"FollowTheWallsCruise",
	"FollowTheWallsStepBack",
	"FollowTheWallsTurnRight",
	"RivalPuckStepBack",
	"RivalPuckTurnRight"
};
#endif