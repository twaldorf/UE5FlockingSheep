# UE5FlockingSheep
Sheep flocking character controller, from MechJam 2023 gamejam in Unreal Engine 5.2.

## Requirements
Compared to a vanilla flocking algorithm, where boids would be in constant movement, it is important that the sheep clump together and seek rest, only moving when other sheep are eaten by wolves or when other sheep are in flight mode. This allows the sheep to regroup and create surviving flocks when dispersed by a wolf. 

The player then gathers these disparate flocks with their mech and returns them to the pen.

## Logic demo
![sheep1](./sheep1.gif)
![sheep2](./sheep2.gif)
![sheep3](./sheep3.gif)
