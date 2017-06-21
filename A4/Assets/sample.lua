
stone = gr.material({0.8, 0.7, 0.7}, {0.0, 0.0, 0.0}, 0)
grass = gr.material({0.1, 0.7, 0.1}, {0.0, 0.0, 0.0}, 0)
hide = gr.material({0.84, 0.6, 0.53}, {0.3, 0.3, 0.3}, 20)

mat1 = gr.material({0.7, 1.0, 0.7}, {0.5, 0.7, 0.5}, 25)
mat3 = gr.material({1.0, 0.6, 0.1}, {0.5, 0.7, 0.5}, 25)
mat4 = gr.material({0.7, 0.6, 1.0}, {0.5, 0.4, 0.8}, 25)

-- #############################################
-- Read in the cow model from a separate file.
-- #############################################


-- #############################################
-- Read in the cow model from a separate file.
-- #############################################

cow_poly = gr.mesh('cow', 'cow.obj')
factor = 2.0/(2.76+3.637)

cow_poly:set_material(hide)

cow_poly:translate(0.0, 3.637, 0.0)
cow_poly:scale(factor, factor, factor)
cow_poly:translate(0.0, -1.0, 0.0)

-- ##############################################
-- the scene
-- ##############################################

scene = gr.node('scene')
scene:rotate('X', 23)

-- the floor
--plane = gr.mesh('plane', 'plane.obj' )
--scene:add_child(plane)
--plane:set_material(grass)
--plane:scale(30, 30, 30)

cow_number = 1

for _, pt in pairs({
		      {{1,1.3, 5}, 20},
		      {{1,5.3,5}, 60},
		      {{1.5,10.3,5}, 120}}) do
   cow_instance = gr.node('cow' .. tostring(cow_number))
   scene:add_child(cow_instance)
   cow_instance:add_child(cow_poly)
   cow_instance:scale(1.4, 1.4, 1.4)
   cow_instance:rotate('Z', pt[2])
   cow_instance:translate(table.unpack(pt[1]))
   
   cow_number = cow_number + 1
end

s_orig = gr.sphere('s_orig')
scene:add_child(s_orig)
s_orig:set_material(mat3)
s_orig:translate(1, 1, 15)
--s_orig:rotate('Y', 60)

tempSphere = s_orig
for i = 1,6 do
    s = gr.sphere('s'..tostring(i))
    s:set_material(mat3)
    s:rotate('Y', 60)
    s:translate(7, 0, -5)
    tempSphere:add_child(s)
    tempSphere = s
end

buckyball = gr.mesh( 'buckyball', 'buckyball.obj' )
scene:add_child(buckyball)
buckyball:set_material(mat1)
buckyball:scale(1.5, 1.5, 1.5)
buckyball:translate(1, -15, 0)

gr.render(scene,
	  'sample.png', 256, 256,
	  {0, 0, 30}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {gr.light({0, -400, 400}, {0.8, 0.8, 0.8}, {1, 0, 0}),
      gr.light({1, -7, 0}, {0.5, 0.5, 1.0}, {1, 0, 0})})