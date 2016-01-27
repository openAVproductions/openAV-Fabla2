/*
 * Author:  Nigel Redmon on 12/18/12.
 * Adapted: Harry van Haaren 2013
 *          harryhaaren@gmail.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//
//  Original work granted under permissive license below:
//
//  Created by Nigel Redmon on 12/18/12.
//  EarLevel Engineering: earlevel.com
//  Copyright 2012 Nigel Redmon
//
//  For a complete explanation of the ADSR envelope generator and code,
//  read the series of articles by the author, starting here:
//  http://www.earlevel.com/main/2013/06/01/envelope-generators/
//
//  License:
//
//  This source code is provided as is, without warranty.
//  You may copy and distribute verbatim copies of this document.
//  You may modify and use this source code to create binary code for your own purposes, free or commercial.
//

#ifndef ADSR_H
#define ADSR_H


class ADSR
{
public:
	ADSR();
	~ADSR();
	float process(void);
	float getOutput(void);
	int getState(void);
	void gate(int on);
	void setAttackRate(float rate);
	void setDecayRate(float rate);
	void setReleaseRate(float rate);
	void setSustainLevel(float level);
	void setTargetRatioA(float targetRatio);
	void setTargetRatioDR(float targetRatio);
	void reset(void);

	enum envState {
		ENV_IDLE = 0,
		ENV_ATTACK,
		ENV_DECAY,
		ENV_SUSTAIN,
		ENV_RELEASE,
	};

protected:
	int state;
	float output;
	float attackRate;
	float decayRate;
	float releaseRate;
	float attackCoef;
	float decayCoef;
	float releaseCoef;
	float sustainLevel;
	float targetRatioA;
	float targetRatioDR;
	float attackBase;
	float decayBase;
	float releaseBase;

	float calcCoef(float rate, float targetRatio);
};

inline float ADSR::process()
{
	switch (state) {
	case ENV_IDLE:
		break;
	case ENV_ATTACK:
		output = attackBase + output * attackCoef;
		if (output >= 1.0) {
			output = 1.0;
			state = ENV_DECAY;
		}
		break;
	case ENV_DECAY:
		output = decayBase + output * decayCoef;
		if (output <= sustainLevel) {
			output = sustainLevel;
			state = ENV_SUSTAIN;
		}
		break;
	case ENV_SUSTAIN:
		break;
	case ENV_RELEASE:
		output = releaseBase + output * releaseCoef;
		if (output <= 0.0) {
			output = 0.0;
			state = ENV_IDLE;
		}
	}
	return output;
}

inline void ADSR::gate(int gate)
{
	if (gate)
		state = ENV_ATTACK;
	else if (state != ENV_IDLE)
		state = ENV_RELEASE;
}

inline int ADSR::getState()
{
	return state;
}

inline void ADSR::reset()
{
	state = ENV_IDLE;
	output = 0.0;
}

inline float ADSR::getOutput()
{
	return output;
}

#endif // ASDR_H
