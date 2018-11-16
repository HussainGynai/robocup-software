#include "WorldBall.hpp"
#include <iostream>
#include <cmath>

void WorldBall::createConfiguration(Configuration* cfg) {
    ball_merger_power = new ConfigDouble("VisionFilter/WorldBall/ball_merger_power", 1.5);
}

WorldBall::WorldBall(std::vecotr<KalmanBall> kalmanBalls) {
    Geometry2d::Point posAvg = Geometry2d::Point(0, 0);
    Geometry2d::Point velAvg = Geometry2d::Point(0, 0);
    double totalPosWeight = 0;
    double totalVelWeight = 0;

    // Below 1 would invert the ratio of scaling
    // Above 2 would just be super noisy
    if (ball_merger_power < 1 || ball_merger_power > 2) {
        std::cout
             << "CRITICAL ERROR: ball_merger_power must be between 1 and 2"
             << std::endl;
    }

    if (kalmanBalls.size() == 0) {
        std::cout
             << "CRITICAL ERROR: Zero balls are given to the WorldBall constructor"
             << std::endl;

        pos = posAvg;
        vel = velAvg;
        posCov = 0;
        velCov = 0;

        return;
    }

    for (KalmanBall& ball : kalmanBalls) {
        // Get the covariance of everything
        // AKA how well we can predict the next measurement
        Geometry2d::Point posCov = ball.getPosCov();
        Geometry2d::Point velCov = ball.getVelCov();

        // Std dev of each state
        // Lower std dev gives better idea of true values
        Geometry2d::Point posStdDev;
        Geometry2d::Point velStdDev;
        posStdDev.x() = std::sqrt(posCov.x());
        posStdDev.y() = std::sqrt(posCov.y());
        velStdDev.x() = std::sqrt(velCov.x());
        velStdDev.y() = std::sqrt(velCov.y());

        // Inversely proportional to how much the filter has been updated
        double filterUncertantity = 1 / ball.getHealth();

        // How good of pos/vel estimation in total
        // (This is less efficient than just doing the sqrt(x_cov + y_cov),
        //  but it's a little more clear math-wise)
        double posUncertantity = posStdDev.mag();
        double velUncertantity = velStdDev.mag();

        double filterPosWeight = std::pow(posUncertantity * filterUncertantity,
                                          -*ball_merger_power);

        double filterVelWeight = std::pow(velUncertantity * filterUncertantity,
                                          -*ball_merger_power);

        posAvg += filterPosWeight * ball.getPos();
        velAvg += filterVelWeight * ball.getVel();

        totalPosWeight += filterPosWeight;
        totalVelWeight += filterVelWeight;
    }

    posAvg /= totalPosWeight;
    velAvg /= totalVelWeight;

    pos = posAvg;
    vel = velAvg;
    posCov = totalPosWeight / kalmanBalls.size();
    velCov = totalVelWeight / kalmanBalls.size();
    ballComponents = kalmanBalls;
}

Geometry2d::Point WorldBall::getPos() {
    return pos;
}

Geometry2d::Point WorldBall::getVel() {
    return vel;
}

double WorldBall::getPosCov() {
    return posCov;
}

double WorldBall::getVelCov() {
    return velCov;
}

std::vector<KalmanBall> getBallComponents() {
    return ballComponents;
}