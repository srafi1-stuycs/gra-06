#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "matrix.h"

void add_box(struct matrix * edges, double x, double y, double z, double width, double height, double depth) {
    add_edge(edges, x, y, z, x + width, y, z);
    add_edge(edges, x + width, y, z, x + width, y, z - depth);
    add_edge(edges, x + width, y, z - depth, x, y, z - depth);
    add_edge(edges, x, y, z - depth, x, y, z);
    add_edge(edges, x, y, z, x, y - height, z);
    add_edge(edges, x + width, y, z, x + width, y - height, z);
    add_edge(edges, x + width, y, z - depth, x + width, y - height, z - depth);
    add_edge(edges, x, y, z - depth, x, y - height, z - depth);
    add_edge(edges, x, y - height, z - depth, x, y - height, z);
    add_edge(edges, x, y - height, z, x + width, y - height, z);
    add_edge(edges, x + width, y - height, z, x + width, y - height, z - depth);
    add_edge(edges, x + width, y - height, z - depth, x, y - height, z - depth);
}

void add_sphere(struct matrix * edges, double cx, double cy, double cz, double r, int steps) {
    struct matrix * sphere_points = new_matrix(4, 4);
    double phi, theta;
    double phi_step = 2 * M_PI / steps;
    double theta_step = M_PI / steps;
    int i, j;
    for (i = 1; i <= steps; i++) {
        phi = i * phi_step;
        for (j = 1; j <= steps; j++) {
            theta = j * theta_step;
            add_point(sphere_points, r * cos(theta) + cx, r * sin(theta) * cos(phi) + cy, r * sin(theta) * sin(phi) + cz);
        }
    }

    double x, y, z;
    for (i = 0; i < sphere_points->lastcol; i++) {
        x = sphere_points->m[0][i];
        y = sphere_points->m[1][i];
        z = sphere_points->m[2][i];
        add_edge(edges, x, y, z, x + 1, y + 1, z + 1);
    }
    free_matrix(sphere_points);
}

void add_torus(struct matrix * edges, double cx, double cy, double cz, double r1, double r2, int steps) {
    struct matrix * torus_points = new_matrix(4, 4);
    double phi, theta;
    double phi_step = 2 * M_PI / steps;
    int i, j;
    for (i = 1; i <= steps; i++) {
        phi = i * phi_step;
        for (j = 1; j <= steps; j++) {
            theta = j * phi_step;
            add_point(torus_points, cos(phi) * (r1 * cos(theta) + r2) + cx, r1 * sin(theta) + cy, -1 * sin(phi) * (r1 * cos(theta) + r2) + cz);
        }
    }

    double x, y, z;
    for (i = 0; i < torus_points->lastcol; i++) {
        x = torus_points->m[0][i];
        y = torus_points->m[1][i];
        z = torus_points->m[2][i];
        add_edge(edges, x, y, z, x + 1, y + 1, z + 1);
    }
    free_matrix(torus_points);
}

void add_circle(struct matrix * edges, double cx, double cy, double cz, double r, double step) {
    double t;
    double x0, y0, x1, y1;
    x0 = r + cx;
    y0 = cy;
    for (t = step; t <= 1; t += step) {
        //printf("t = %lf\n", t);
        x1 = r*cos(2*M_PI*t) + cx;
        y1 = r*sin(2*M_PI*t) + cy;
        add_edge(edges, x0, y0, cz, x1, y1, cz);
        x0 = x1;
        y0 = y1;
    }
}

void add_curve(struct matrix * edges, double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3, double step, int type) {
    double t;
    double segment_x0, segment_y0, segment_x1, segment_y1;

    struct matrix * x_coefs = generate_curve_coefs(x0, x1, x2, x3, type);
    double ax = x_coefs->m[0][0];
    double bx = x_coefs->m[1][0];
    double cx = x_coefs->m[2][0];
    double dx = x_coefs->m[3][0];
    free_matrix(x_coefs);

    struct matrix * y_coefs = generate_curve_coefs(y0, y1, y2, y3, type);
    double ay = y_coefs->m[0][0];
    double by = y_coefs->m[1][0];
    double cy = y_coefs->m[2][0];
    double dy = y_coefs->m[3][0];
    free_matrix(y_coefs);

    segment_x0 = dx;
    segment_y0 = dy;

    for (t = step; t <= 1 + step; t += step) {
        segment_x1 = t*t*t*ax + t*t*bx + t*cx + dx;
        segment_y1 = t*t*t*ay + t*t*by + t*cy + dy;
        add_edge(edges, segment_x0, segment_y0, 0, segment_x1, segment_y1, 0);
        segment_x0 = segment_x1;
        segment_y0 = segment_y1;
    }
}

void add_point( struct matrix * points, double x, double y, double z) {

    if ( points->lastcol == points->cols )
        grow_matrix( points, points->lastcol + 100 );

    points->m[0][ points->lastcol ] = x;
    points->m[1][ points->lastcol ] = y;
    points->m[2][ points->lastcol ] = z;
    points->m[3][ points->lastcol ] = 1;
    points->lastcol++;
} //end add_point

void add_edge( struct matrix * points, 
        double x0, double y0, double z0, 
        double x1, double y1, double z1) {
    add_point( points, x0, y0, z0 );
    add_point( points, x1, y1, z1 );
}

void draw_lines( struct matrix * points, screen s, color c) {

    int point;
    for (point=0; point < points->lastcol-1; point+=2)
        draw_line( points->m[0][point],
                points->m[1][point],
                points->m[0][point+1],
                points->m[1][point+1],
                s, c);	       
}// end draw_lines

void draw_line(int x0, int y0, int x1, int y1, screen s, color c) {

    int x, y, d, A, B;
    //swap points if going right -> left
    int xt, yt;
    if (x0 > x1) {
        xt = x0;
        yt = y0;
        x0 = x1;
        y0 = y1;
        x1 = xt;
        y1 = yt;
    }

    x = x0;
    y = y0;
    A = 2 * (y1 - y0);
    B = -2 * (x1 - x0);  

    //octants 1 and 8
    if ( abs(x1 - x0) >= abs(y1 - y0) ) {

        //octant 1    
        if ( A > 0 ) {

            d = A + B/2;      
            while ( x < x1 ) {
                plot( s, c, x, y );
                if ( d > 0 ) {
                    y+= 1;
                    d+= B;
                }
                x++;
                d+= A;
            } //end octant 1 while
            plot( s, c, x1, y1 );
        } //end octant 1

        //octant 8
        else {
            d = A - B/2;

            while ( x < x1 ) {
                //printf("(%d, %d)\n", x, y);
                plot( s, c, x, y );
                if ( d < 0 ) {
                    y-= 1;
                    d-= B;
                }
                x++;
                d+= A;
            } //end octant 8 while
            plot( s, c, x1, y1 );
        } //end octant 8
    }//end octants 1 and 8

    //octants 2 and 7
    else {

        //octant 2    
        if ( A > 0 ) {
            d = A/2 + B;      

            while ( y < y1 ) {
                plot( s, c, x, y );
                if ( d < 0 ) {
                    x+= 1;
                    d+= A;
                }
                y++;
                d+= B;
            } //end octant 2 while
            plot( s, c, x1, y1 );
        } //end octant 2

        //octant 7
        else {
            d = A/2 - B;

            while ( y > y1 ) {
                plot( s, c, x, y );
                if ( d > 0 ) {
                    x+= 1;
                    d+= A;
                }
                y--;
                d-= B;
            } //end octant 7 while
            plot( s, c, x1, y1 );
        } //end octant 7   
    }//end octants 2 and 7  
} //end draw_line
