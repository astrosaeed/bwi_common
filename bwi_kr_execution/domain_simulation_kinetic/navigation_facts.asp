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
room    (s16).
room    (s17).
room    (s18).
room    (s19).
room    (s20).
room    (s21).
room    (s22).
room    (s23).
room    (s24).
room    (s25).

% naming d1 for door on level 1. location where this door can be found after underscore
door(d0).
door(d1).
door(d2).
door(d3).
door(d4).
door(d5).

hasdoor(s1, d0).
hasdoor(s2, d1).
hasdoor(s3, d0).
hasdoor(s4, d1).
hasdoor(s8, d2).
hasdoor(s9, d2).
hasdoor(s11, d3).
hasdoor(s12, d3).
hasdoor(s16, d4).
hasdoor(s17, d4).
hasdoor(s19, d5).
hasdoor(s20, d5).

% Accecibility

acc(s0, s1). acc(s0, s2). acc(s1, s2).
acc(s3, s4). acc(s3, s5). acc(s3, s6). acc(s3, s7). acc(s3, s8). acc(s3, s10). acc(s3, s11). acc(s3, s17). acc(s3, s20). acc(s3, s21). acc(s3, s22). acc(s3, s23). acc(s3, s24). acc(s3, s25).
acc(s4, s5). acc(s4, s6). acc(s4, s7). acc(s4, s8). acc(s4, s10). acc(s4, s11). acc(s4, s17). acc(s4, s20). acc(s4, s21). acc(s4, s22). acc(s4, s23). acc(s4, s24). acc(s4, s25).
acc(s5, s6). acc(s5, s7). acc(s5, s8). acc(s5, s10). acc(s5, s11). acc(s5, s17). acc(s5, s20). acc(s5, s21). acc(s5, s22). acc(s5, s23). acc(s5, s24). acc(s5, s25).
acc(s6, s7). acc(s6, s8). acc(s6, s10). acc(s6, s11). acc(s6, s17). acc(s6, s20). acc(s6, s21). acc(s6, s22). acc(s6, s23). acc(s6, s24). acc(s6, s25).
acc(s7, s8). acc(s7, s10). acc(s7, s11). acc(s7, s17). acc(s7, s20). acc(s7, s21). acc(s7, s22). acc(s7, s23). acc(s7, s24). acc(s7, s25).
acc(s8, s10). acc(s8, s11). acc(s8, s17). acc(s8, s20). acc(s8, s21). acc(s8, s22). acc(s8, s23). acc(s8, s24). acc(s8, s25).
acc(s10, s11). acc(s10, s17). acc(s10, s20). acc(s10, s21). acc(s10, s22). acc(s10, s23). acc(s10, s24). acc(s10, s25).
acc(s11, s17). acc(s11, s20). acc(s11, s21). acc(s11, s22). acc(s11, s23). acc(s11, s24). acc(s11, s25).
acc(s17, s20). acc(s17, s21). acc(s17, s22). acc(s17, s23). acc(s17, s24). acc(s17, s25).
acc(s20, s21). acc(s20, s22). acc(s20, s23). acc(s20, s24). acc(s20, s25).
acc(s21, s22). acc(s21, s23). acc(s21, s24). acc(s21, s25).
acc(s22, s23). acc(s22, s24). acc(s22, s25).
acc(s23, s24). acc(s23, s25).
acc(s24, s25).
acc(s9, s12). acc(s9, s13). acc(s9, s14). acc(s9, s15). acc(s9, s16). acc(s9, s18). acc(s9, s19).
acc(s12, s13). acc(s12, s14). acc(s12, s15). acc(s12, s16). acc(s12, s18). acc(s12, s19).
acc(s13, s14). acc(s13, s15). acc(s13, s16). acc(s13, s18). acc(s13, s19).
acc(s14, s15). acc(s14, s16). acc(s14, s18). acc(s14, s19).
acc(s15, s16). acc(s15, s18). acc(s15, s19).
acc(s16, s18). acc(s16, s19).
acc(s18, s19).

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
location(l6).                                                         
inside(l6, s6).
location(l7).                                                         
inside(l7, s7).
location(l0).                                                         
inside(l10, s10).
location(l13).                                                         
inside(l13, s13).
location(l14).                                                         
inside(l14, s14).
location(l15).                                                         
inside(l15, s15).
location(l18).                                                         
inside(l18, s18).
location(l0).                                                         
inside(l21, s21).
location(l22).                                                         
inside(l22, s22).
location(l23).                                                         
inside(l23, s23).
location(l24).                                                         
inside(l24, s24).
location(l25).                                                         
inside(l25, s25).