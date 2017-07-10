-- Simple Scene:
-- An extremely simple scene that will render right out of the box
-- with the provided skeleton code.  It doesn't rely on hierarchical
-- transformations.

-- Create the top level root node named 'root'.
rootNode = gr.node('root')

<<<<<<< HEAD

bcube = gr.mesh('cube', 'bcube', 1, 1,1)
bcube:scale(1.0, 1.0, 1.0)
bcube:translate(-0.5, 0.0, 10.0)
bcube:set_material(gr.material({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 100.0))
rootNode:add_child(bcube)

bcube = gr.mesh('cube', 'bcube', 1, 1,1)
bcube:scale(1.0, 1.0, 1.0)
bcube:translate(0.5, 0.0, 9.0)
bcube:set_material(gr.material({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 100.0))
rootNode:add_child(bcube)
=======
blue = gr.material({0.7, 0.6, 1}, {0.5, 0.4, 0.8}, 25)
white = gr.material({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 100.0)
black = gr.material({0.2, 0.2, 0.2}, {0.1,0.1,0.1}, 10)
>>>>>>> 17568f7bee4dfd6e60cc70b11354a944260f9146
-- | |
for i = 1,7 do
    cube = gr.mesh('cube', 'Lcube'..tostring(i), 1, 1,1)
    cube:scale(1.0, 1.0, 1.0)
    cube:translate(-1.5, 0.0, 9.0 - i - i*0.1)
    cube:set_material(white)
    rootNode:add_child(cube)
end

for i = 1,7 do
    cube = gr.mesh('cube', 'Rcube'..tostring(i), 1, 1,1)
    cube:scale(1.0, 1.0, 1.0)
    cube:translate(1.8, 0.0, 9.0 - i - i*0.1)
    cube:set_material(white)
    rootNode:add_child(cube)
end

-- __   __
--   | |
for i = 1,7 do
    cube = gr.mesh('cube', 'LBcube'..tostring(i), 1, 1,1)
    cube:translate(-1.5 - i - i*0.1, 0.0, 9.0 - 7 - 7*0.1)
    cube:set_material(white)
    rootNode:add_child(cube)
end

for i = 1,7 do
    cube = gr.mesh('cube', 'RBcube'..tostring(i), 1, 1,1)
    cube:translate(1.8 + i + i*0.1, 0.0, 9.0 - 7 - 7*0.1)
    cube:set_material(white)
    rootNode:add_child(cube)
end

--|       |
--|__   __|
--   | |
for i = 1,20 do
    cube = gr.mesh('cube', 'LLcube'..tostring(i), 1, 1,1)
    cube:translate(-9.2, 0.0, 1.3 - i - i*0.1)
    cube:set_material(white)
    rootNode:add_child(cube)
end

for i = 1,20 do
    cube = gr.mesh('cube', 'RRcube'..tostring(i), 1,1,1)
    cube:translate(9.5, 0.0, 1.3 - i - i*0.1)
    cube:set_material(white)
    rootNode:add_child(cube)
end

-- _______
--|       |
--|__   __|
--   | |
for i = 1,18 do
    cube = gr.mesh('cube', 'Tcube'..tostring(i),1,1,1)
    cube:translate(-9.2 + (i -1) + (i-1)*0.1, 0.0, 1.3 - 21 - 21*0.1)
    cube:set_material(white)
    rootNode:add_child(cube)
end

obstacle = gr.mesh('cube', 'obstacle', 1, 1,1)
obstacle:scale(1.0, 1.0, 1.0)
obstacle:translate(-2, 0.0, -5)
obstacle:set_material(gr.material({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 100.0))
rootNode:add_child(obstacle)

obstacle2 = gr.mesh('cube', 'obstacle', 1, 1,1)
obstacle2:scale(1.0, 1.0, 1.0)
obstacle2:translate(2, 0.0, -5)
obstacle2:set_material(gr.material({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 100.0))
rootNode:add_child(obstacle2)

plane = gr.mesh('plane', 'plane',2.0, 0.1, 2.0)
plane:scale(12, 1.0, 20)
plane:translate(0.0, -0.5, -5.0)
plane:set_material(white)
rootNode:add_child(plane)

player = gr.mesh('player', 'player', 0.5, 0.1, 1.0)
player:scale(1.0, 1.0, 1.0)
<<<<<<< HEAD
player:translate(0.2, 0.3, 7.6)
player:set_material(gr.material({0.1, 0.1, 0.1}, {0.1,0.1,0.1}, 10))
=======
player:translate(0.2, 0.5, 7.6)
player:set_material(white)
>>>>>>> 17568f7bee4dfd6e60cc70b11354a944260f9146
rootNode:add_child(player)

enemy1 = gr.mesh('sphere', 'e1', 0.7,0.7,0.7)
enemy1:scale(0.7,0.7,0.7)
enemy1:translate(5, 0.2, -15)
enemy1:set_material(black)
rootNode:add_child(enemy1)

enemy2 = gr.mesh('sphere', 'e2',1.0, 1.0, 1.0)
enemy2:scale(0.7, 0.7, 0.7)
enemy2:translate(-5, 0.2, -15)
enemy2:set_material(black)
rootNode:add_child(enemy2)

tsphere = gr.mesh('sphere', 'tsphere', 2.0, 2.0, 2.0)
tsphere:scale(2.0, 2.0, 2.0)
tsphere:translate(-5, 0.2, -3)
tsphere:set_material(blue)
rootNode:add_child(tsphere)

tsphere2 = gr.mesh('sphere', 'tsphere2', 2.0, 2.0, 2.0)
tsphere2:scale(2.0, 2.0, 2.0)
tsphere2:translate(-5, 0.2, -3)
tsphere2:set_material(blue)
rootNode:add_child(tsphere2)

-- Return the root with all of it's childern.  The SceneNode Project::m_rootNode will be set
-- equal to the return value from this Lua script.
return rootNode