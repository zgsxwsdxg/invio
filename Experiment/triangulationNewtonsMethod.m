syms r00 r01 r02 r10 r11 r12 r20 r21 r22 r0 r1 r2 ux uy
syms x real
syms y real
syms z real
syms t real

Rt = [r00, r01, r02, r0;
      r10, r11, r12, r1;
      r20, r21, r22, r2]
X = [t*x, t*y, t*z, 1.0]';



f = Rt * (X);

f(1) = (ux - (f(1) / f(3)))^2;
f(2) = (uy - (f(2) / f(3)))^2;

error = f(1) + f(2);

error = expand(error);

error_diff = simplify(diff(error, t), 100);

error_diff2 = simplify(diff(error_diff, t), 100);

delta_x = collect(-1*error_diff / error_diff2, t);

delta_x = subs(delta_x, Rt, [1, 0, 0, 0; 0, 1, 0, 0.01; 0, 0, 1, 0])
delta_x = subs(delta_x, [ux, uy, x, y, z], [0, -0.01, 0, 0, 1])

test = (-(r20*x + r21*y + r22*z)*(r0*r00*r20*x^2 - r2*r10^2*x^2 - r01^2*r2*y^2 - r2*r11^2*y^2 - r02^2*r2*z^2 - r2*r12^2*z^2 - r00^2*r2*x^2 + r1*r10*r20*x^2 + r0*r01*r21*y^2 + r1*r11*r21*y^2 + r0*r02*r22*z^2 + r1*r12*r22*z^2 - r0*r20^2*ux*x^2 - r1*r20^2*uy*x^2 - r0*r21^2*ux*y^2 - r1*r21^2*uy*y^2 - r0*r22^2*ux*z^2 - r1*r22^2*uy*z^2 + r00*r2*r20*ux*x^2 + r2*r10*r20*uy*x^2 + r01*r2*r21*ux*y^2 + r2*r11*r21*uy*y^2 + r02*r2*r22*ux*z^2 + r2*r12*r22*uy*z^2 - 2*r00*r01*r2*x*y + r0*r00*r21*x*y + r0*r01*r20*x*y - 2*r2*r10*r11*x*y + r1*r10*r21*x*y + r1*r11*r20*x*y - 2*r00*r02*r2*x*z + r0*r00*r22*x*z + r0*r02*r20*x*z - 2*r2*r10*r12*x*z + r1*r10*r22*x*z + r1*r12*r20*x*z - 2*r01*r02*r2*y*z + r0*r01*r22*y*z + r0*r02*r21*y*z - 2*r2*r11*r12*y*z + r1*r11*r22*y*z + r1*r12*r21*y*z + r00*r2*r21*ux*x*y + r01*r2*r20*ux*x*y - 2*r0*r20*r21*ux*x*y + r2*r10*r21*uy*x*y + r2*r11*r20*uy*x*y - 2*r1*r20*r21*uy*x*y + r00*r2*r22*ux*x*z + r02*r2*r20*ux*x*z - 2*r0*r20*r22*ux*x*z + r2*r10*r22*uy*x*z + r2*r12*r20*uy*x*z - 2*r1*r20*r22*uy*x*z + r01*r2*r22*ux*y*z + r02*r2*r21*ux*y*z - 2*r0*r21*r22*ux*y*z + r2*r11*r22*uy*y*z + r2*r12*r21*uy*y*z - 2*r1*r21*r22*uy*y*z)*t^2 + (2*ux*r0*r2*r20^2*x^2 - 2*r0^2*r20*r21*x*y - 2*r0^2*r20*r22*x*z - r0^2*r21^2*y^2 - 2*r0^2*r21*r22*y*z - r0^2*r22^2*z^2 - r0^2*r20^2*x^2 + 4*ux*r0*r2*r20*r21*x*y + 4*ux*r0*r2*r20*r22*x*z + 2*ux*r0*r2*r21^2*y^2 + 4*ux*r0*r2*r21*r22*y*z + 2*ux*r0*r2*r22^2*z^2 + r00^2*r2^2*x^2 + 2*r00*r01*r2^2*x*y + 2*r00*r02*r2^2*x*z - 2*ux*r00*r2^2*r20*x^2 - 2*ux*r00*r2^2*r21*x*y - 2*ux*r00*r2^2*r22*x*z + r01^2*r2^2*y^2 + 2*r01*r02*r2^2*y*z - 2*ux*r01*r2^2*r20*x*y - 2*ux*r01*r2^2*r21*y^2 - 2*ux*r01*r2^2*r22*y*z - r1^2*r20^2*x^2 - 2*r1^2*r20*r21*x*y - 2*r1^2*r20*r22*x*z - r1^2*r21^2*y^2 - 2*r1^2*r21*r22*y*z - r1^2*r22^2*z^2 + 2*uy*r1*r2*r20^2*x^2 + 4*uy*r1*r2*r20*r21*x*y + 4*uy*r1*r2*r20*r22*x*z + 2*uy*r1*r2*r21^2*y^2 + 4*uy*r1*r2*r21*r22*y*z + 2*uy*r1*r2*r22^2*z^2 + r02^2*r2^2*z^2 - 2*ux*r02*r2^2*r20*x*z - 2*ux*r02*r2^2*r21*y*z - 2*ux*r02*r2^2*r22*z^2 + r2^2*r10^2*x^2 + 2*r2^2*r10*r11*x*y + 2*r2^2*r10*r12*x*z - 2*uy*r2^2*r10*r20*x^2 - 2*uy*r2^2*r10*r21*x*y - 2*uy*r2^2*r10*r22*x*z + r2^2*r11^2*y^2 + 2*r2^2*r11*r12*y*z - 2*uy*r2^2*r11*r20*x*y - 2*uy*r2^2*r11*r21*y^2 - 2*uy*r2^2*r11*r22*y*z + r2^2*r12^2*z^2 - 2*uy*r2^2*r12*r20*x*z - 2*uy*r2^2*r12*r21*y*z - 2*uy*r2^2*r12*r22*z^2)*t + r0*r00*r2^2*x + r1*r2^2*r10*x - r0^2*r2*r20*x - r1^2*r2*r20*x + r0*r01*r2^2*y + r1*r2^2*r11*y - r0^2*r2*r21*y - r1^2*r2*r21*y + r0*r02*r2^2*z + r1*r2^2*r12*z - r0^2*r2*r22*z - r1^2*r2*r22*z - r00*r2^3*ux*x - r2^3*r10*uy*x - r01*r2^3*ux*y - r2^3*r11*uy*y - r02*r2^3*ux*z - r2^3*r12*uy*z + r0*r2^2*r20*ux*x + r1*r2^2*r20*uy*x + r0*r2^2*r21*ux*y + r1*r2^2*r21*uy*y + r0*r2^2*r22*ux*z + r1*r2^2*r22*uy*z)/((-2*(r20*x + r21*y + r22*z)*(r0*r00*r20*x^2 - r2*r10^2*x^2 - r01^2*r2*y^2 - r2*r11^2*y^2 - r02^2*r2*z^2 - r2*r12^2*z^2 - r00^2*r2*x^2 + r1*r10*r20*x^2 + r0*r01*r21*y^2 + r1*r11*r21*y^2 + r0*r02*r22*z^2 + r1*r12*r22*z^2 - r0*r20^2*ux*x^2 - r1*r20^2*uy*x^2 - r0*r21^2*ux*y^2 - r1*r21^2*uy*y^2 - r0*r22^2*ux*z^2 - r1*r22^2*uy*z^2 + r00*r2*r20*ux*x^2 + r2*r10*r20*uy*x^2 + r01*r2*r21*ux*y^2 + r2*r11*r21*uy*y^2 + r02*r2*r22*ux*z^2 + r2*r12*r22*uy*z^2 - 2*r00*r01*r2*x*y + r0*r00*r21*x*y + r0*r01*r20*x*y - 2*r2*r10*r11*x*y + r1*r10*r21*x*y + r1*r11*r20*x*y - 2*r00*r02*r2*x*z + r0*r00*r22*x*z + r0*r02*r20*x*z - 2*r2*r10*r12*x*z + r1*r10*r22*x*z + r1*r12*r20*x*z - 2*r01*r02*r2*y*z + r0*r01*r22*y*z + r0*r02*r21*y*z - 2*r2*r11*r12*y*z + r1*r11*r22*y*z + r1*r12*r21*y*z + r00*r2*r21*ux*x*y + r01*r2*r20*ux*x*y - 2*r0*r20*r21*ux*x*y + r2*r10*r21*uy*x*y + r2*r11*r20*uy*x*y - 2*r1*r20*r21*uy*x*y + r00*r2*r22*ux*x*z + r02*r2*r20*ux*x*z - 2*r0*r20*r22*ux*x*z + r2*r10*r22*uy*x*z + r2*r12*r20*uy*x*z - 2*r1*r20*r22*uy*x*z + r01*r2*r22*ux*y*z + r02*r2*r21*ux*y*z - 2*r0*r21*r22*ux*y*z + r2*r11*r22*uy*y*z + r2*r12*r21*uy*y*z - 2*r1*r21*r22*uy*y*z))*t - 3*r0^2*r20^2*x^2 - 6*r0^2*r20*r21*x*y - 6*r0^2*r20*r22*x*z - 3*r0^2*r21^2*y^2 - 6*r0^2*r21*r22*y*z - 3*r0^2*r22^2*z^2 + 4*r0*r00*r2*r20*x^2 + 4*r0*r00*r2*r21*x*y + 4*r0*r00*r2*r22*x*z + 4*r0*r01*r2*r20*x*y + 4*r0*r01*r2*r21*y^2 + 4*r0*r01*r2*r22*y*z + 4*r0*r02*r2*r20*x*z + 4*r0*r02*r2*r21*y*z + 4*r0*r02*r2*r22*z^2 + 2*ux*r0*r2*r20^2*x^2 + 4*ux*r0*r2*r20*r21*x*y + 4*ux*r0*r2*r20*r22*x*z + 2*ux*r0*r2*r21^2*y^2 + 4*ux*r0*r2*r21*r22*y*z + 2*ux*r0*r2*r22^2*z^2 - r00^2*r2^2*x^2 - 2*r00*r01*r2^2*x*y - 2*r00*r02*r2^2*x*z - 2*ux*r00*r2^2*r20*x^2 - 2*ux*r00*r2^2*r21*x*y - 2*ux*r00*r2^2*r22*x*z - r01^2*r2^2*y^2 - 2*r01*r02*r2^2*y*z - 2*ux*r01*r2^2*r20*x*y - 2*ux*r01*r2^2*r21*y^2 - 2*ux*r01*r2^2*r22*y*z - 3*r1^2*r20^2*x^2 - 6*r1^2*r20*r21*x*y - 6*r1^2*r20*r22*x*z - 3*r1^2*r21^2*y^2 - 6*r1^2*r21*r22*y*z - 3*r1^2*r22^2*z^2 + 4*r1*r2*r10*r20*x^2 + 4*r1*r2*r10*r21*x*y + 4*r1*r2*r10*r22*x*z + 4*r1*r2*r11*r20*x*y + 4*r1*r2*r11*r21*y^2 + 4*r1*r2*r11*r22*y*z + 4*r1*r2*r12*r20*x*z + 4*r1*r2*r12*r21*y*z + 4*r1*r2*r12*r22*z^2 + 2*uy*r1*r2*r20^2*x^2 + 4*uy*r1*r2*r20*r21*x*y + 4*uy*r1*r2*r20*r22*x*z + 2*uy*r1*r2*r21^2*y^2 + 4*uy*r1*r2*r21*r22*y*z + 2*uy*r1*r2*r22^2*z^2 - r02^2*r2^2*z^2 - 2*ux*r02*r2^2*r20*x*z - 2*ux*r02*r2^2*r21*y*z - 2*ux*r02*r2^2*r22*z^2 - r2^2*r10^2*x^2 - 2*r2^2*r10*r11*x*y - 2*r2^2*r10*r12*x*z - 2*uy*r2^2*r10*r20*x^2 - 2*uy*r2^2*r10*r21*x*y - 2*uy*r2^2*r10*r22*x*z - r2^2*r11^2*y^2 - 2*r2^2*r11*r12*y*z - 2*uy*r2^2*r11*r20*x*y - 2*uy*r2^2*r11*r21*y^2 - 2*uy*r2^2*r11*r22*y*z - r2^2*r12^2*z^2 - 2*uy*r2^2*r12*r20*x*z - 2*uy*r2^2*r12*r21*y*z - 2*uy*r2^2*r12*r22*z^2)

test = subs(test, Rt, [1, 0, 0, 0; 0, 1, 0, 0.01; 0, 0, 1, 0])
test = subs(test, [ux, uy, x, y, z], [0, -0.01, 0, 0, 1])
test = subs(test, t, 2)


