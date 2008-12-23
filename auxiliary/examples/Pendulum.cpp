#include "SimTKsimbody.h"
#include "SimTKsimbody_aux.h"

using namespace SimTK;

int main() {
    
    // Create the system.
    
    MultibodySystem system;
    SimbodyMatterSubsystem matter(system);
    GeneralForceSubsystem forces(system);
    Force::UniformGravity gravity(forces, matter, Vec3(0, -9.8, 0));
    Body::Rigid pendulumBody(MassProperties(1.0, Vec3(0), Inertia(1)));
    pendulumBody.addDecoration(Transform(), DecorativeSphere(0.1));
    MobilizedBody::Pin pendulum(matter.updGround(), Transform(Vec3(0)), pendulumBody, Transform(Vec3(0, 1, 0)));
    system.updDefaultSubsystem().addEventReporter(new VTKEventReporter(system, 0.01));
    
    // Initialize the system and state.
    
    system.realizeTopology();
    State state = system.getDefaultState();
    pendulum.setOneU(state, 0, 1.0);
    
    // Simulate it.

    RungeKuttaMersonIntegrator integ(system);
    TimeStepper ts(system, integ);
    ts.initialize(state);
    ts.stepTo(100.0);

    return 0;
}