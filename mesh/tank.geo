#
## a cylinder
#
algebraic3d

# 5 - 5 - 0.025


solid sp1 = sphere (0, 0, 0.0895; 2.000) and plane (0,0,1.3745;0,0,-1) and not sphere (0, 0, 0.0895; 1.99765);	
solid sp2 = sphere (0, 0, -0.0895; 2.000) and plane (0,0,-1.3745;0,0,1) and not sphere (0, 0, -0.0895; 1.99765);	
solid sp3 = sphere (0, 0, -2.661; 2.000) and plane (0,0,-3.96;0,0,1) and not sphere (0, 0, -2.661; 1.99765);	


solid sp_1 = sphere (0, 0, 0.75; 1.5) and plane (0, 0, 0.75; 0, 0, -1) and not sphere (0, 0, 0.75; 1.49715);	


solid cyl= (cylinder ( 0, 0, -4.06; 0, 0, 1.4745; 1.500 ) 
        and plane (0,0,-4.06;0,0,-1)
        and plane (0, 0, 1.4745; 0, 0, 1)
        and not 
        (
           cylinder ( 0, 0, -4.055; 0, 0, 1.4695; 1.49715)
           and plane (0,0,-4.055;0,0,-1)
           and plane (0, 0, 1.4695; 0, 0, 1)
        ))
        and not (sphere (0, 0, 0.0895; 1.99765) and not plane (0,0,1.4145;0,0,1))
        and not (sphere (0, 0, -2.661; 1.99765) and not plane (0,0,-3.99;0,0,-1)) ;

solid cyl1= cylinder ( 0, 0, -2.05; 0, 0, -4.7; 0.2 )
        and plane (0,0,-2.05;0,0,1)
        and plane (0, 0, -4.7; 0, 0, -1);

solid cyl2 = cylinder ( 0, 0, 0; 0, 0, -5.5; 0.198 )
        and plane (0,0,1;0,0,1);


	
solid shell = ( sp1  or sp_1   ) and plane (0,0,0;1,0,0) and plane(0,0,0;0,1,0);


#solid shell = (((sp_1) or  (sp1 ))  ) and plane (0,0,0;1,0,0) and plane(0,0,0;0,1,0);

#solid shell = (cyl or sp1 or sp_1) and plane (0,0,0;1,0,0) and plane(0,0,0;0,1,0) and not (cylinder ( 0, 0, -3; 0, 0, 3; 2.500 ) and not cylinder ( 0, 0, -3; 0, 0, 3; 1.500 ));

#solid shell = (((cyl  or sp1 or sp2 or sp3)  or cyl1) and not cyl2 ) and plane (0,0,0;1,0,0) and plane(0,0,0;0,1,0) and not (cylinder ( 0, 0, -3; 0, 0, 3; 2.500 ) and not cylinder ( 0, 0, -3; 0, 0, 3; 1.500 ));


#solid shell = (((cyl  or sp1 or sp2 or sp3)  or cyl1) and not cyl2 )  and not (cylinder ( 0, 0, -3; 0, 0, 3; 2.500 ) and not cylinder ( 0, 0, -3; 0, 0, 3; 1.500 ));
#solid shell = ( (((cyl or sp1 or sp2 or sp3)  or cyl1) and not cyl2) and not cyl2) and plane (0,0,0;1,0,0);




tlo shell;
