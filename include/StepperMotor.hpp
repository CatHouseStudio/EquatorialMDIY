#ifndef StepperMotor_h
#define StepperMotor_h

class  StepperMotor {
    public:
        StepperMotor(int stepPin, int dirPin);
        void step(int steps, bool direction, int speed);
        void rotateWithEarth();
        void setGearRatio(float newRatio);
        void stop();
        void reset();
    private:
        int _stepPin, _dirPin;
        float _gearRatio;

    
};

#endif