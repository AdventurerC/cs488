-- puppet.lua
-- A simplified puppet without posable joints, but that
-- looks roughly humanoid.

rootnode = gr.node('root')
rootnode:rotate('y', -20.0)
rootnode:scale( 0.25, 0.25, 0.25 )
rootnode:translate(0.0, 0.0, -1.0)

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
red2 = gr.material({0.7, 0.0, 0.0}, {0.1, 0.1, 0.1}, 20)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
green2 = gr.material({0.0, 0.7, 0.0}, {0.1, 0.1, 0.1}, 20)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)

torsoTop = gr.mesh('cube', 'torsoTop')
rootnode:add_child(torsoTop)
torsoTop:set_material(green)
torsoTop:scale(0.5,0.5,0.5);

waist = gr.joint('waist', {-90, 0.0, 0}, {-45, 0, -45})
torsoTop:add_child(waist)
waist:translate(0, -0.5, 0)

torso = gr.mesh('cube', 'torso')
waist:add_child(torso)
torso:set_material(green2)
torso:scale(1.0,1.0,1.0);
torso:translate(0,-0.5,0);

neck = gr.mesh('sphere', 'neck')
torsoTop:add_child(neck)
neck:scale(1.0/0.5, 2.0, 1.0/0.5)
neck:scale(0.15, 0.3, 0.15)
neck:translate(0.0, 0.6, 0.0)
neck:set_material(blue)

neckJoint = gr.joint('neckJoint', {-30, 0.0, 30}, {-90, 0.0, 90})
--neckJoint:scale(1.0/0.5, 1.0, 1.0/0.5)
--neckJoint:scale(0.4, 0.4, 0.4)
--neckJoint:translate(0.0, 0.9, 0.0)
neck:add_child(neckJoint)

head = gr.mesh('cube', 'head')
neckJoint:add_child(head)
head:scale(3, 1.5, 3)
--head:scale(0.4, 0.4, 0.4)
head:translate(0.0, 0.9, 0.0)
head:set_material(red)

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
torsoTop:add_child(leftShoulder)
leftShoulder:scale(1/0.5,1.0,1/0.5);
leftShoulder:scale(0.2, 0.4, 0.2)
leftShoulder:translate(-0.4, 0.2, 0.0)
leftShoulder:set_material(blue)

-- which = -1 ==> left; 1 ==> right
function genArm (which, name, elbowName, handName)
    arm = gr.mesh('cube', name)
    --torso:add_child(arm)
    arm:scale(1/0.5, 1.0, 1/0.5);
    arm:scale(1, 0.5, 0.5)
    arm:rotate('z', 50*which*-1);
    arm:translate(which*1, -0.3, 0.0)
    arm:set_material(red)

    arm2elbow = gr.joint(name..'-elbow', {-45, 0, 45}, {-45, 0, 45})
    arm:add_child(arm2elbow)
    
    elbow = gr.mesh('cube', elbowName)
    arm2elbow:add_child(elbow)
    elbow:scale(1/0.5, 1/0.5, 1/0.5);
    elbow:scale(0.4, 0.4, 0.4)
    elbow:rotate('z', 50*which*-1);
    elbow:translate(which*0.8, 0.0, 0.1)
    elbow:set_material(red)

    elbow2hand = gr.joint(elbowName..'-hand', {-45, 0, 45}, {-45, 0, 45})
    elbow:add_child(elbow2hand)
    elbow2hand:translate(which*0.8,0.5, 0.2)

    hand = gr.mesh('cube', handName)
    elbow2hand:add_child(hand)
    hand:scale(0.5, 0.5, 0.5)
    hand:rotate('x', 50*which*-1);
    --hand:translate(which*0.8,0.5, 0.2)
    hand:set_material(red)

    return arm
end

leftArm = genArm(-1, 'leftArm', 'leftElbow', 'leftHand')
rightArm = genArm(1, 'rightArm', 'rightElbow', 'rightHand')

--torso:add_child(leftArm)
--torso:add_child(rightArm)

armJointL = gr.joint('armJointL', {-90, 0, 90}, {-10, 0, 10})
leftShoulder:add_child(armJointL)
armJointL:add_child(leftArm)

rightShoulder = gr.mesh('sphere', 'rightShoulder')
torsoTop:add_child(rightShoulder)
rightShoulder:scale(1/0.5,1.0,1/0.5);
rightShoulder:scale(0.2, 0.4, 0.2)
rightShoulder:translate(0.4, 0.2, 0.0)
rightShoulder:set_material(blue)

armJointR = gr.joint('armJointR', {-90, 0, 90}, {-10, 0, 10})
rightShoulder:add_child(armJointR)
armJointR:add_child(rightArm)


leftHip = gr.mesh('sphere', 'leftHip')
torso:add_child(leftHip)
leftHip:scale(1/0.5,2.0,1/0.5);
leftHip:scale(0.21, 0.21, 0.21)
leftHip:translate(-0.38, -0.5, 0.0)
leftHip:set_material(blue)

leftHipJoint = gr.joint('leftHipJoint', {-90, 0, 90}, {-10, 0, 10})
leftHip:add_child(leftHipJoint)

rightHip = gr.mesh('sphere', 'rightHip')
torso:add_child(rightHip)
rightHip:scale(1/0.5,2.0,1/0.5);
rightHip:scale(0.21, 0.21, 0.21)
rightHip:translate(0.38, -0.5, 0.0)
rightHip:set_material(blue)

--rightHipJoint
rightHipJoint = gr.joint('rightHipJoint', {-90, 0, 90}, {-10, 0, 10})
rightHip:add_child(rightHipJoint)

leftLeg = gr.mesh('cube', 'leftLeg')
leftHipJoint:add_child(leftLeg)
leftLeg:scale(0.5,2,0.5)
leftLeg:translate(0,-1.5,0)
leftLeg:set_material(red)

--leftKneeJoint
leftKneeJoint = gr.joint('leftKneeJoint', {-90, 0, 90}, {-10, 0, 10})
leftLeg:add_child(leftKneeJoint)
leftKneeJoint:scale(1/0.5, 0.5,1/0.5)
leftKneeJoint:translate(0, -1, 0)


rightLeg = gr.mesh('cube', 'rightLeg')
rightHipJoint:add_child(rightLeg)
rightLeg:scale(0.5,2,0.5)
rightLeg:translate(0,-1.5,0)
rightLeg:set_material(red)

--rightKneeJoint
rightKneeJoint = gr.joint('rightKneeJoint', {-90, 0, 90}, {-10, 0, 10})
rightLeg:add_child(rightKneeJoint)
rightKneeJoint:scale(1/0.5, 0.5,1/0.5)
rightKneeJoint:translate(0,-1,0)


leftLeg2 = gr.mesh('cube', 'leftLeg2')
leftKneeJoint:add_child(leftLeg2)
--leftLeg2:scale(0.5,2,0.5)
leftLeg2:scale(0.5,2,0.5)
leftLeg2:translate(0,0,0)
leftLeg2:set_material(red2)


rightLeg2 = gr.mesh('cube', 'rightLeg2')
rightKneeJoint:add_child(rightLeg2)
--rightLeg2:scale(0.5,2,0.5)
rightLeg2:scale(0.5,2,0.5)
rightLeg2:translate(0,0,0)
rightLeg2:set_material(red2)


--add feet
rightAnkle = gr.joint('rightAnkle', {-90, 0, 90}, {-10, 0, 10})
rightLeg2:add_child(rightAnkle);
rightAnkle:translate(0,-0.5,0)

leftAnkle = gr.joint('leftAnkle', {-90, 0, 90}, {-10, 0, 10})
leftLeg2:add_child(leftAnkle);
leftAnkle:translate(0,-0.5,0)

leftFoot = gr.mesh('cube', 'leftFoot')
leftAnkle:add_child(leftFoot)
leftFoot:scale(1.5, 0.5, 1.5)


rightFoot = gr.mesh('cube', 'rightFoot')
rightAnkle:add_child(rightFoot)
rightFoot:scale(1.5, 0.5, 1.5)
--rightFoot:translate(1, 0, 0)
--rightFoot:rotate('x', 1)

return rootnode
