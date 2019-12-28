#program base.
room(n09).
room(t01).
room(main_office).
room(shiqi_office).

room(n_corr).
room(t_corr).
room(p_corr).
room(q_corr).
room(s_corr).
room(r_corr).
room(main_corr).


door(n09_d1).
door(n09_d2).
door(shiqi_office_d).
door(n_corr_d1).
door(n_corr_d2).
door(q_corr_d1).
door(q_corr_d2).
door(t01_d1).
door(t01_d2).
door(main_office_d).
door(s_corr_d1).
door(t_corr_d1).
door(t_corr_d2).

hasdoor(n09,n09_d1). 
hasdoor(n09,n09_d2).  
hasdoor(n_corr,n09_d1). 
hasdoor(n_corr,n09_d2).  

hasdoor(n_corr,shiqi_office_d).  
hasdoor(shiqi_office,shiqi_office_d).  

hasdoor(n_corr,n_corr_d1).  
hasdoor(main_corr,n_corr_d1).  


hasdoor(n_corr,n_corr_d2).  
hasdoor(main_corr,n_corr_d2).  


hasdoor(q_corr,q_corr_d1).  
hasdoor(main_corr,q_corr_d1).  


hasdoor(q_corr,q_corr_d2).  
hasdoor(main_corr,q_corr_d2).  


hasdoor(s_corr,s_corr_d1).  
hasdoor(main_corr,s_corr_d1).


hasdoor(t_corr,t_corr_d1).  
hasdoor(main_corr,t_corr_d1).
  
hasdoor(t_corr,t_corr_d2).  
hasdoor(main_corr,t_corr_d2).

hasdoor(t01,t01_d1).  
hasdoor(main_corr,t01_d1).  


hasdoor(t01,t01_d2).  
hasdoor(main_corr,t01_d2).  


hasdoor(main_office,main_office_d).  
hasdoor(main_corr,main_office_d).  

acc(n_corr, p_corr).
acc(p_corr, q_corr).


dooracc(R1,D,R2) :- hasdoor(R1,D), hasdoor(R2,D), R1 != R2, door(D), room(R1), room(R2).
dooracc(R1,D,R2) :- dooracc(R2,D,R1).

acc(R1,R1) :- room(R1).                                                         
acc(R1,R2) :- acc(R2,R1), room(R1), room(R2).                                   
acc(R1,R2) :- acc(R1,R3), acc(R2,R3), room(R1), room(R2), room(R3).             

%Not on the map
%object(coffee_counter).                                                         
%inside(coffee_counter, l2_302).


object(water_fountain).
inside(water_fountain,q_corr).




%badDoor(d3_414a3).
%badDoor(d3_414b3).
object(logo).
inside(logo,main_corr).
