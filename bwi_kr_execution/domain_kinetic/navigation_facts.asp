#program base.

room    (s0).
room    (s1).
room    (s2).
room    (s3).
room    (s4).
room    (s5).
room    (s6).
room    (s7).
room    (s8).
room    (s9).
room    (s10).
room    (s11).
room    (s12).
room    (s13).
room    (s14).
room    (s15).

% naming d1 for door on level 1. location where this door can be found after underscore
door(d0).
door(d1).
door(d2).
door(d3).
door(d4).

hasdoor(s1, d0).
hasdoor(s2, d1).
hasdoor(s3, d0).
hasdoor(s4, d1).
hasdoor(s9, d2).
hasdoor(s10, d2).
hasdoor(s6, d3).
hasdoor(s7, d3).
hasdoor(s12, d4).
hasdoor(s13, d4).

% Accecibility

acc(s0, s1). acc(s0, s2). acc(s1, s2).
acc(s5, s3). acc(s5, s6). acc(s5, s8).
acc(s7, s15).
acc(s8, s4). acc(s8, s9). acc(s8, s11).
acc(s10, s15).
acc(s11, s12).
acc(s13, s14).
acc(s14, s15).


dooracc(R1,D,R2) :- hasdoor(R1,D), hasdoor(R2,D), R1 != R2, door(D), room(R1), room(R2).
dooracc(R1,D,R2) :- dooracc(R2,D,R1).

acc(R1,R1) :- room(R1).                                                         
acc(R1,R2) :- acc(R2,R1), room(R1), room(R2).                                   
acc(R1,R2) :- acc(R1,R3), acc(R2,R3), room(R1), room(R2), room(R3).  

%Not on the map
location(l0).                                                         
inside(l0, s0).
location(l5).                                                         
inside(l5, s5).
location(l8).                                                         
inside(l8, s8).
location(l11).                                                         
inside(l11, s11).
location(l14).                                                         
inside(l14, s14).
location(l15).                                                         
inside(l15, s15).
