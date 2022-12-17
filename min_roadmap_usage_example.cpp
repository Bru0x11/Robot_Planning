#include "VisiLibity1/src/visilibity.hpp"
#include <vector>
#include <iostream>
#include <cfloat>
#include "dubins.h"
#include "math.h"



#include <fstream>

using namespace VisiLibity;
using namespace std;

class Vector
{
public:
    double x;
    double y;

    Vector(double x, double y)
    {
        // Constructor
        this->x = x;
        this->y = y;
    }

    double norm()
    {
        double n = sqrt(pow(this->x, 2) + pow(this->y, 2));
        return n;
    }
};

double compute_m(Point p0, Point p1){

    //cout<<"sto calcolando m"<<endl;

    double x1 = p1.x();
    double x0 = p0.x();

    //cout<<"x0: "<<x0<<endl;
    //cout<<"x1: "<<x1<<endl;

    if ((x1-x0) != 0)
    {
        double y1,y0;
        y1 = p1.y();
        y0 = p0.y();
        //cout<<"y0: "<<y0<<endl;
        //cout<<"y1: "<<y1<<endl;
        return (y1-y0) / (x1-x0);
    }
    return DBL_MAX;
}

double find_distance(Point p0, Point p1, Point p2, double minR)
{
    double m1, m2;
    // Compute m
    m1 = compute_m(p0,p1);

    m2 = compute_m(p1,p2);

    double tan_angle = (m1 - m2) / (1 + (m1 * m2));
    double angle = atan(tan_angle);

    double distance = minR * (1 / tan(angle / 2));

    return abs(distance);
}

// given the initial and final points of the segment and a distance, compute the entrance point
Point find_entrance(Point p0, Point p1, double distance)
{
    double x0 = p0.x();
    double y0 = p0.y();
    double x1 = p1.x();
    double y1 = p1.y();

    Vector v = Vector(x1 - x0, y1 - y0);
    double n = v.norm();

    double unit_v_x = v.x / n;
    double unit_v_y = v.y / n;

    Vector unit_v = Vector(unit_v_x, unit_v_y);

    double x_entrance = x0 + distance * unit_v.x;
    double y_entrance = y0 + distance * unit_v.y;
    Point entrance = Point(x_entrance, y_entrance);

    return entrance;
};

Point find_exit(Point p0, Point p1, double distance)
{
    Point exit = find_entrance(p1, p0, distance);
    return exit;
}


Polyline get_points_from_arc(Arc a, int npts){
    
    Polyline points;
    Point point;
    Pos pose;

    for (int j=0; j<npts; j++){

        double s = a.L/npts * j;

        pose = circline(s, a.x0, a.y0, a.th0, a.k);

        point.set_x(pose.x);
        point.set_y(pose.y);

        points.push_back(point);
    }

    return points; 
}

Polyline get_points_from_curve(Curve c, int npts){

    int npts_arc = (int)(npts/3); 

    Arc a1 = c.a1;
    Arc a2 = c.a2;
    Arc a3 = c.a3;

    Polyline line1 = get_points_from_arc(a1, npts_arc);
    Polyline line2 = get_points_from_arc(a2, npts_arc);
    Polyline line3 = get_points_from_arc(a3, npts_arc);
    
    Polyline tot_line;
    tot_line.append(line1);
    tot_line.append(line2);
    tot_line.append(line3);

    return tot_line;
}

Polyline get_points_line(Point p0, Point p1){
    
    Polyline points;
    double m = compute_m(p0, p1);
    double q = p0.y() - (m*p0.x());
    
    cout<<"costruisco segment da punto "<<p0<<endl;
    cout<<"a punto "<<p1<<endl;

    cout<<"m: "<<m<<endl;
    cout<<"q: "<<q<<endl;
    double x0 = p0.x();
    double x1 = p1.x();

    if(x1>x0){
        for(int i=1; i<100; i++){
            double gamma = i*0.01;
            cout<<"gamma: "<<gamma<<endl;
            double temp_x = (1-gamma)*x0 + (gamma*x1);
            cout<<"temp_x: "<<temp_x<<endl;
            double temp_y = m*temp_x + q;
            cout<<"temp_y: "<<temp_y<<endl;

            Point p = Point(temp_x, temp_y);

            points.push_back(p);
        }
    }

    return points;

}


double compute_arc_length(Point a, Point b, double minR){
    
    double points_distance = sqrt(pow(b.x() - a.x(),2) + pow(b.y() - a.y(),2));
    double central_angle = acos(1 - (pow(points_distance,2)/(2*pow(minR, 2))));

    double L = minR * central_angle;

    return L;
}

Arc get_arc(Point a, Point b, Point c, double minR){
    
    double distance = find_distance(a, b, c, minR);
    Point exit = find_exit(a, b, distance);
    Point entrance = find_entrance(b, c, distance);

    double m1 = compute_m(a,b);
    double m2 = compute_m(b,c);

    double angle1 = atan(m1);
    double angle2 = atan(m2);
    
    Arc arc;

    arc.x0 = entrance.x();
    arc.y0 = entrance.y();
    arc.th0 = angle1;
    arc.xf = exit.x();
    arc.yf = exit.y();
    arc.thf = angle2;
    arc.k = 1/minR;
    arc.L = compute_arc_length(entrance, exit, minR);

    return arc;

}

int main()
{

    // Create and open a text file
    ofstream MyFile("env.csv");
    MyFile<<"x,y"<<endl;

    vector<Point> points_obs1;
    points_obs1.push_back(Point(1.0, 2.0));
    points_obs1.push_back(Point(6.0, 7.0));
    points_obs1.push_back(Point(6.0, 2.0));


    //for (Point point : points_obs1){
    //   MyFile<<point.x()<<","<<point.y()<<endl;
    //}

    Polygon obs1 = Polygon(points_obs1);

    vector<Point> points_obs2;
    points_obs2.push_back(Point(2.0, 9.0));
    points_obs2.push_back(Point(2.0, 14.0));
    points_obs2.push_back(Point(8.0, 14.0));
    points_obs2.push_back(Point(8.0, 9.0));

    //for (Point point : points_obs2){
    //    MyFile<<point.x()<<","<<point.y()<<endl;
    //}

    Polygon obs2 = Polygon(points_obs2);

    vector<Point> points_env;
    points_env.push_back(Point(0.0, 0.0));
    points_env.push_back(Point(15.0, 0.0));
    points_env.push_back(Point(15.0, 15.0));
    points_env.push_back(Point(0.0, 15.0));

    //for (Point point : points_env){
    //    MyFile<<point.x()<<" "<<point.y()<<endl;
    //}

    Polygon poly_env = Polygon(points_env);

    // cout<<obs2.area();

    vector<Polygon> obstacles;
    obstacles.push_back(poly_env);
    obstacles.push_back(obs1);
    obstacles.push_back(obs2);

    Environment env = Environment(obstacles);

    Visibility_Graph graph = Visibility_Graph(env, 0);

    Point start_test = Point(3.0, 1.0);
    Point end = Point(9.0, 14.0);
    Polyline shortest_path = env.shortest_path(start_test, end, graph, 0.0);

    // cout<<shortest_path.length()<<endl;

    cout << "Enviroment is valid: " << env.is_valid() << endl;
    // cout<<"\n"<<env<<endl;

    cout << "Shortest_path: " << endl;
    cout << shortest_path << endl;


    for (int i = 0; i<shortest_path.size(); i++){
        MyFile<<shortest_path[i].x()<<","<<shortest_path[i].y()<<endl;
    }


    // for Point in shortest path
    int path_length = shortest_path.size();


    Polyline points_final_path;


    //BUILD DUBINS FIRST TRAIT
    //first 3 vertex (need them to do dubins)
    Point start = shortest_path[0];
    Point p0 = shortest_path[1];
    Point p1 = shortest_path[2];

    //for(int i = 0; i<shortest_path.size()-2; i++){
    //    Point p0 = shortest_path[i];
    //    Point p1 = shortest_path[i+1];
    //
    //    Polyline line_points = get_points_line(p0, p1);
    //    points_final_path.append(line_points);
    //}

    double th0 = 0;
    //Find final angle of first trait
    double m = compute_m(p0,p1);
    double th1 = atan(m);

    double minR = 1;
    double Kmax = 1/minR;
    Curve first_trait = dubins_shortest_path(start.x(), start.y(), th0, p0.x(), p0.y(), th1, Kmax);
    Polyline points_first_trait = get_points_from_curve(first_trait, 300);
    points_final_path.append(points_first_trait);




    //BUILD VECTOR WITH ALL ARCS
    vector<Arc> arc_vector;
    for (int i = 0; i < path_length - 2; i++)
    {
        Point a = shortest_path[i];
        Point b = shortest_path[i+1];
        Point c = shortest_path[i+2];

        Arc arc = get_arc(a,b,c, minR);
        arc_vector.push_back(arc);
    }


    //BUILD DUBINS LAST TRAIT
    //last 3 vertex (need them to build dubins last trait)
    Point pn_1 = shortest_path[path_length-3];
    Point pn = shortest_path[path_length-2];
    Point goal = shortest_path[path_length-1];

    double thf = 0;
    double m_final_segment = compute_m(pn,goal);
    double thn = atan(m_final_segment);

    Curve last_trait = dubins_shortest_path(goal.x(), goal.y(), th0, pn.x(), pn.y(), thf, Kmax);
    Polyline points_last_trait = get_points_from_curve(last_trait, 300);
    //points_final_path.append(points_last_trait);


    //DO FIRST LINE (from second vertex (i.e. where dubins arrives to third one))
    Point p_second = shortest_path[]


    for(int i = 0; i<arc_vector.size(); i++){
        Arc arc = arc_vector[i];
        Polyline arc_points = get_points_from_arc(arc, 100);
        points_final_path.append(arc_points);
    }




    for(int i=0; i<points_final_path.size(); i++){
        double x = points_final_path[i].x();
        double y = points_final_path[i].y();
        MyFile<<x<<","<<y<<endl;
    }


    // Close the file
    MyFile.close();

    // cout<<"poses[0]: "<<poses<<endl;

    return 0;
}