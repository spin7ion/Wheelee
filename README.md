Wheelee
=======

Arduino inverted pendulum robot that use LQR control.
**Author:** Alexander Semion

Mathematical model
------------------

The model development is described in [article(Russian)](http://spin7ion.ru/ru/blog/balancerBuildSteps).
The model was created in Matlab Simulink it is saved in balancer.slx.

Hardware
--------

Robot uses
* L3G gyroscope
* LSM303 accelerometer and magnetometer
* DualVNH5019 Motor Shield
* Motors with encoders
* HC-05 bluetooth

Arduino sketch
--------------

The arduino sketch is located in sketch folder.


Documentation
-------------

* [mathematical model development(Russian)](http://spin7ion.ru/ru/blog/balancerBuildSteps)
* [building of robot(Russian)](http://habrahabr.ru/post/220989)
