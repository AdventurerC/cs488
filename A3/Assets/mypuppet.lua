-- puppet.lua
-- A simplified puppet without posable joints, but that
-- looks roughly humanoid.

rootnode = gr.node('root')
rootnode:rotate('y', -20.0)
rootnode:scale( 0.25, 0.25, 0.25 )
rootnode:translate(0.0, 0.0, -1.0)

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)

torso = gr.mesh('cube', 'torso')
rootnode:add_child(torso)
torso:set_material(white)
torso:scale(0.5,1.0,0.5);

head = gr.mesh('cube', 'head')
torso:add_child(head)
head:scale(1.0/0.5, 1.0, 1.0/0.5)
head:scale(0.4, 0.4, 0.4)
head:translate(0.0, 0.9, 0.0)
head:set_material(red)

neck = gr.mesh('sphere', 'neck')
torso:add_child(neck)
neck:scale(1.0/0.5, 1.0, 1.0/0.5)
neck:scale(0.15, 0.3, 0.15)
neck:translate(0.0, 0.6, 0.0)
neck:set_material(blue)

neckJoint = gr.joint('neckJoint', {-30, 0.0, 30}, {-90, 0.0, -80})
neck:add_child(neckJoint)

ears = gr.mesh('sphere', 'ears')
head:add_child(ears)
ears:scale(1.2, 0.08, 0.08)
ears:set_material(red)
ears:set_material(blue)

leftEye = gr.mesh('cube', 'leftEye')
head:add_child(leftEye)
leftEye:scale(0.2, 0.1, 0.1)
leftEye:translate(-0.2, 0.2, 0.5)
leftEye:set_material(blue)

rightEye = gr.mesh('cube', 'rightEye')
head:add_child(rightEye)
rightEye:scale(0.2, 0.1, 0.1)
rightEye:translate(0.2, 0.2, 0.5)
rightEye:set_material(blue)

leftShoulder = gr.mesh('sphere', 'leftShoulder')
torso:add_child(leftShoulder)
leftShoulder:scale(1/0.5,1.0,1/0.5);
leftShoulder:scale(0.2, 0.2, 0.2)
leftShoulder:translate(-0.4, 0.35, 0.0)
leftShoulder:set_material(blue)

-- which = -1 ==> left; 1 ==> right
function genArm (which, name, elbowName, handName)
    arm = gr.mesh('cube', name)
    --torso:add_child(arm)
    arm:scale(1/0.5, 1.0, 1/0.5);
    arm:scale(0.2, 0.1, 0.1)
    arm:rotate('z', 50*which*-1);
    arm:translate(which*0.8, 0.1, 0.0)
    arm:set_material(red)

    arm2elbow = gr.joint(name..'-elbow', {0, 0, which*1.5}, {0, 0, which*1.5})
    arm:add_child(arm2elbow)
    
    elbow = gr.mesh('cube', elbowName)
    arm2elbow:add_child(elbow)
    elbow:scale(1/0.5, 1/0.5, 1/0.5);
    elbow:scale(0.4, 0.4, 0.4)
    elbow:rotate('z', 50*which*-1);
    elbow:translate(which*0.8, 0.0, 0.1)
    elbow:set_material(red)

    elbow2hand = gr.joint(elbowName..'-hand', {0, 0, which*1.5}, {0, 0, which*1.5})
    elbow:add_child(elbow2hand)

    hand = gr.mesh('sphere', handName)
    elbow2hand:add_child(hand)
    hand:scale(0.5, 0.5, 0.5)
    hand:rotate('x', 50*which*-1);
    hand:translate(which*0.8,0.1, 0.2)
    hand:set_material(red)

    return arm
end

leftArm = genArm(-1, 'leftArm', 'leftElbow', 'leftHand')
rightArm = genArm(1, 'rightArm', 'rightElbow', 'rightHand')

torso:add_child(leftArm)
torso:add_child(rightArm)

armJointL = gr.joint('armJointL', {-90, 0, -90}, {-10, 0, -10})
leftShoulder:add_child(armJointL)

rightShoulder = gr.mesh('sphere', 'rightShoulder')
torso:add_child(rightShoulder)
rightShoulder:scale(1/0.5,1.0,1/0.5);
rightShoulder:scale(0.2, 0.2, 0.2)
rightShoulder:translate(0.4, 0.35, 0.0)
rightShoulder:set_material(blue)

armJointR = gr.joint('armJointR', {-90, 0, -90}, {-10, 0, -10})
leftShoulder:add_child(armJointR)

leftHip = gr.mesh('sphere', 'leftHip')
torso:add_child(leftHip)
leftHip:scale(1/0.5,1.0,1/0.5);
leftHip:scale(0.21, 0.21, 0.21)
leftHip:translate(-0.38, -0.5, 0.0)
leftHip:set_material(blue)

--leftHipJoint

rightHip = gr.mesh('sphere', 'rightHip')
torso:add_child(rightHip)
rightHip:scale(1/0.5,1.0,1/0.5);
rightHip:scale(0.21, 0.21, 0.21)
rightHip:translate(0.38, -0.5, 0.0)
rightHip:set_material(blue)

--rightHipJoint

leftLeg = gr.mesh('cube', 'leftLeg')
leftHip:add_child(leftLeg)
leftLeg:scale(0.5,4,0.5)
leftLeg:translate(0,-2.8,0)
leftLeg:set_material(red)

--leftKneeJoint

rightLeg = gr.mesh('cube', 'rightLeg')
rightHip:add_child(rightLeg)
rightLeg:scale(0.5,4,0.5)
rightLeg:translate(0,-2.8,0)
rightLeg:set_material(red)

--rightKneeJoint

--add feet
return rootnode
