#include "cgal_iso.h"
#include <commons/obanal/tfield.h>
#include <commons/obanal/tisolinedata.h>
#include <commons/obanal/func_obanal.h>
#include <qvector3d.h>


#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/convex_hull_2.h>
#include <CGAL/Alpha_shape_2.h>
#include <vector>

typedef CGAL::Exact_predicates_inexact_constructions_kernel     K;
typedef CGAL::Triangulation_vertex_base_with_info_2<int, K>     Vb;
typedef CGAL::Triangulation_data_structure_2<Vb>                Tds;
typedef CGAL::Delaunay_triangulation_2<K, Tds>                  DT;

typedef K::FT FT;
typedef K::Segment_2  Segment;
typedef K::Vector_2  Vector_2;
typedef K::Point_2  Point_2;

typedef DT::Point                                               Point;
typedef std::vector<DT::Point>                                  PointVector;
typedef DT::Vertex_handle                                       Vertex_handle;
typedef DT::Face_handle                                         Face_handle;
typedef DT::Vertex_circulator                                   Vertex_circulator;
typedef DT::Face_circulator                                     Face_circulator;
typedef DT::Edge_circulator                                     Edge_circulator;
typedef DT::Edge_iterator Ei;

typedef CGAL::Exact_predicates_tag                               Itag;
typedef CGAL::Constrained_Delaunay_triangulation_2<K, CGAL::Default, Itag> CDT;
typedef CDT::Vertex_handle                                       CVertex_handle;


typedef CGAL::Alpha_shape_vertex_base_2<K> aVb;
typedef CGAL::Alpha_shape_face_base_2<K>  aFb;
typedef CGAL::Triangulation_data_structure_2<aVb,aFb> aTds;
typedef CGAL::Delaunay_triangulation_2<K,aTds> aTriangulation_2;
typedef CGAL::Alpha_shape_2<aTriangulation_2>  Alpha_shape_2;
typedef Alpha_shape_2::Alpha_shape_edges_iterator Alpha_shape_edges_iterator;
typedef Alpha_shape_2::Vertex_handle                                       AVertex_handle;


#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Gps_circle_segment_traits_2.h>
#include <CGAL/General_polygon_set_2.h>
#include <CGAL/Lazy_exact_nt.h>
#include <CGAL/Boolean_set_operations_2.h>


typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
//typedef Kernel::Point_2                                   Point_2;
typedef Kernel::Circle_2                                  Circle_2;
typedef CGAL::Gps_circle_segment_traits_2<Kernel>         Traits_2;

typedef CGAL::General_polygon_set_2<Traits_2>             Polygon_set_2;
typedef Traits_2::General_polygon_2                       Polygon_2;
typedef Traits_2::General_polygon_with_holes_2            Polygon_with_holes_2;
typedef Traits_2::Curve_2                                 Curve_2;
typedef Traits_2::X_monotone_curve_2                      X_monotone_curve_2;

#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Quotient.h>
#include <CGAL/MP_Float.h>
#include <CGAL/Cartesian_converter.h>

typedef Kernel::Point_2                                   ePoint_2;
typedef CGAL::Polygon_2<Kernel>                           ePolygon_2;
typedef CGAL::Polygon_with_holes_2<Kernel>                ePolygon_with_holes_2;

typedef CGAL::Cartesian_converter< Kernel,K > E_I;

typedef CGAL::Cartesian_converter< K, Kernel > I_E;

CgalIso::CgalIso()
{

}

QVector< QPolygon> getTriang(const DT &dt)
{
  //CGAL::refine_Delaunay_mesh_2(dt,Criteria(0.01, 0.125));
  QVector< QPolygon> trret;
  for( DT::Finite_faces_iterator fit = dt.finite_faces_begin();
       fit != dt.finite_faces_end(); ++fit )
  {
    QPolygon triangl;
    Vertex_handle opposite1 = fit->vertex(0);
    QPoint p1 = QPoint(opposite1->point().hx(),opposite1->point().hy());
    triangl<<p1;
    opposite1 = fit->vertex(1);
    p1 = QPoint(opposite1->point().hx(),opposite1->point().hy());
    triangl<<p1;
    opposite1 = fit->vertex(2);
    p1 = QPoint(opposite1->point().hx(),opposite1->point().hy());
    triangl<<p1;
    trret<<triangl;
  }
  return trret;
}

float distance(const meteo::GeoPoint& gp1,const meteo::GeoPoint& gp2)
{
  return gp1.calcDistance(gp2);

}

void getNears(DT *dt, const Vertex_handle &vertex,
              const std::map<Vertex_handle,
              meteo::GeoPoint>& gpm, int num,
              PointVector* points,
              int max_rast,
              std::map<std::pair<Point,Point>, int> *distm)
{
  Vertex_circulator circ = dt->incident_vertices(vertex);
  Vertex_circulator done = circ;
  if (circ != 0 ) {
    do {
      if (circ->info() < 0 ) {
        int dist = gpm.at(vertex).calcDistance(gpm.at(circ));
        const std::pair<Point,Point> &p =std::make_pair(vertex->point(),circ->point());
        distm->insert(std::make_pair(p,dist));
        if(dist <= max_rast )
        {
          circ->info() = num;
          points->push_back(circ->point());
          getNears(dt,circ,gpm,num,points,max_rast,distm);
        }


      }
    }while(++circ != done);
  }
}

bool getAddPoint(const Vertex_handle &p1,
                 const Vertex_handle &p2,
                 const DT &cdt,
                 Vertex_handle *retp)
{
  Face_circulator circ = cdt.incident_faces(p1);
  Face_circulator done = circ;
  QVector<int> nums;
  if (circ != 0 ) {
    do {
      Face_handle face = circ;
      if( true == cdt.is_infinite(face)){
        ++circ;
        continue;
      }
      nums.clear();
      nums<<0<<1<<2;
      if(true == face->has_vertex(p2)){
        nums.removeAll(face->index(p1));
        nums.removeAll(face->index(p2));
        Vertex_handle vh=  face->vertex(nums.first()%3);
        if(false == cdt.is_infinite(vh))
        {
          *retp = vh;
          return true;
        }
      }
    }while(++circ != done);
  }
  return false;
}


bool getNextBorderPoint(std::vector<DT::Segment> *ss,const Point& p1, Point*p2 )
{
  std::vector<DT::Segment>::iterator it = ss->begin();
  std::vector<DT::Segment>::iterator eit = ss->end();
  for(;it!=eit;++it){
    if(it->source() == p1){
      *p2 = it->target();
      ss->erase(it);
      return true;
    }
    if(it->target() == p1){
      *p2 = it->source();
      ss->erase(it);
      return true;
    }
  }
  return false;
}

bool getNextBorderPoints(std::vector<DT::Segment> *ss,const Point& p1,
                         std::vector<Point>* )
{
  Point p2;
  bool res = getNextBorderPoint(ss,p1, &p2);
  //if(!res)
  return res;

  // pnts


}

Polygon_2 construct_polygon (const Circle_2& circle)
{
  // Subdivide the circle into two x-monotone arcs.
  Traits_2 traits;
  Curve_2 curve (circle);
  std::list<CGAL::Object>  objects;
  traits.make_x_monotone_2_object() (curve, std::back_inserter(objects));
  CGAL_assertion (objects.size() == 2);
  // Construct the polygon.
  Polygon_2 pgn;
  X_monotone_curve_2 arc;
  std::list<CGAL::Object>::iterator iter;
  for (iter = objects.begin(); iter != objects.end(); ++iter) {
    CGAL::assign (arc, *iter);
    pgn.push_back (arc);
  }
  return pgn;
}

QPainterPath construct_path(const Polygon_2 &pgn)
{
    QPainterPath result;

    Q_ASSERT(pgn.orientation() == CGAL::CLOCKWISE ||
             pgn.orientation() == CGAL::COUNTERCLOCKWISE);

    // Degenerate polygon, ring.size() < 3
    if (pgn.orientation() == CGAL::ZERO) {
        qWarning() << "construct_path: Ignoring degenerated polygon";
        return  result;
    }

    const bool isClockwise = pgn.orientation() == CGAL::CLOCKWISE;

    Polygon_2::Curve_const_iterator current = pgn.curves_begin();
    Polygon_2::Curve_const_iterator end = pgn.curves_end();

    result.moveTo(CGAL::to_double(current->source().x()),
                  CGAL::to_double(current->source().y()));

    do {
        const Polygon_2::X_monotone_curve_2 &curve = *current;
        const auto &source = curve.source();
        const auto &target = curve.target();

        if (curve.is_linear()) {
            result.lineTo(QPointF(CGAL::to_double(target.x()),
                                  CGAL::to_double(target.y())));
        }
        else if (curve.is_circular()) {
            const auto bbox = curve.supporting_circle().bbox();
            const QRectF rect(QPointF(bbox.xmin(), bbox.ymin()),
                              QPointF(bbox.xmax(), bbox.ymax()));
            const auto center = curve.supporting_circle().center();
            const double asource = atan2(CGAL::to_double(source.y() - center.y()),
                                          CGAL::to_double(source.x() - center.x()));
            const double atarget = atan2(CGAL::to_double(target.y() - center.y()),
                                          CGAL::to_double(target.x() - center.x()));
            double aspan = atarget - asource;
            if (aspan < -CGAL_PI || (qFuzzyCompare(aspan, -CGAL_PI) && !isClockwise))
                aspan += 2.0*CGAL_PI;
            else if (aspan > CGAL_PI || (qFuzzyCompare(aspan, CGAL_PI) && isClockwise))
                aspan -= 2.0*CGAL_PI;
            result.arcTo(rect, MnMath::rad2deg(-asource), MnMath::rad2deg(-aspan));
        }
        else { // ?!?
            Q_UNREACHABLE();
        }
    } while (++current != end);

    return result;
}

#include <CGAL/point_generators_2.h>
typedef CGAL::Random_points_on_circle_2<Point_2> G;

ePolygon_2 construct_polygon1(ePoint_2 center, int radius)
{
  //G pg(radius);
  ePolygon_2 points;
  //points.reserve(100);
  for(int i = 0; i < 360; i+=10){
    ePoint_2 p((cos(2 * M_PI * i / 360) * radius )+ center.x(),
               (sin(2 * M_PI * i / 360) * radius )+ center.y());
    points.push_back(p );
  }
  return points;
}

QPolygonF construct_polygon2(Point center, int radius)
{
  //G pg(radius);
  QPolygonF points;
  //points.reserve(100);
  for(int i = 0; i < 360; i+=10){
    QPointF p((cos(2 * M_PI * i / 360) * radius )+ center.x(),
               (sin(2 * M_PI * i / 360) * radius )+ center.y());
    points.push_back(p );
  }
  return points;
}



void makeJoinCircle(const PointVector &points,std::vector<PointVector> *pv )
{
  std::vector<ePolygon_2> circles;

  for( PointVector::const_iterator pit = points.begin();
       pit != points.end(); ++pit )
  {
    circles.push_back(construct_polygon1(ePoint_2(pit->x(),pit->y()), 6371.*300.));
  }
  std::vector<ePolygon_with_holes_2> res;

  CGAL::join(circles.begin(), circles.end(), std::back_inserter (res));

  std::vector<ePolygon_with_holes_2>::iterator cit = res.begin();
  std::vector<ePolygon_with_holes_2>::iterator ecit = res.end();
  for(;cit!=ecit;++cit){
    ePolygon_2 poly = cit->outer_boundary();
    ePolygon_2::Vertex_iterator it = poly.vertices_begin();
    ePolygon_2::Vertex_iterator eit = poly.vertices_end();
    PointVector apv;
    for(;it!=eit;++it){
      apv.push_back(E_I()(*it));
    }
    pv->push_back(apv);
  }
  // QPainterPath p = construct_path(poly);

}

void makeJoinCircle2(const PointVector &points,std::vector<PointVector> *pv)
{
  float rad = 6371.*300.*6371.*300.;
  std::list<Polygon_2> circles;
  for( PointVector::const_iterator pit = points.begin();
       pit != points.end(); ++pit )
  {
    ePoint_2 ep = I_E()(*pit);
    circles.push_back(construct_polygon(Circle_2(ep, rad)));
  }
  std::vector<Polygon_with_holes_2> res;

  CGAL::join(circles.begin(), circles.end(), std::back_inserter (res));

  std::vector<Polygon_with_holes_2>::iterator cit = res.begin();
  std::vector<Polygon_with_holes_2>::iterator ecit = res.end();
  for(;cit!=ecit;++cit){
    Polygon_2 poly = cit->outer_boundary();

    QPainterPath p = construct_path(poly);
    QList<QPolygonF> qpol = p.toSubpathPolygons();
     qpol = p.toFillPolygons();

    QList<QPolygonF>::iterator it = qpol.begin();
    QList<QPolygonF>::iterator eit = qpol.end();
    PointVector apv;
    for(;it!=eit;++it){
      for(int i= 0; i < it->size(); ++i){
        apv.push_back(Point(it->at(i).x(),it->at(i).y()));
      }
    }
    pv->push_back(apv);
  }

//  qWarning() << "QList<QPolygonF>"<<qpol.size();

}
void makeJoinCircle1(const PointVector &points,std::vector<PointVector> *pv)
{
  float rad = 6371.*300.;
  std::vector<QPolygonF> circles;
  for( PointVector::const_iterator pit = points.begin();
       pit != points.end(); ++pit )
  {
    circles.push_back( construct_polygon2(*pit, rad));
  }
  //std::vector<Polygon_with_holes_2> res;

 // CGAL::join(circles.begin(), circles.end(), std::back_inserter (res));

  std::vector<QPolygonF>::iterator cit = circles.begin();
  std::vector<QPolygonF>::iterator ecit = circles.end();
  QPainterPath p;

  for(;cit!=ecit;++cit){
    // Polygon_2 poly = cit->outer_boundary();
    QPainterPath j;
    j.addPolygon(*cit);
    p = p.united(j);
  }

  QList<QPolygonF> qpol = p.toFillPolygons();

  QList<QPolygonF>::iterator it = qpol.begin();
  QList<QPolygonF>::iterator eit = qpol.end();
  PointVector apv;
  for(;it!=eit;++it){
    for(int i= 0; i < it->size(); ++i){
      apv.push_back(Point(it->at(i).x(),it->at(i).y()));
    }
    pv->push_back(apv);
  }

//  qWarning() << "QList<QPolygonF>"<<qpol.size();

}


void makeHull(const PointVector &points,
              PointVector *hulls)
{
  DT dt(points.begin(),points.end());
  std::vector<DT::Segment> segments;
  for (DT::Finite_edges_iterator eit = dt.finite_edges_begin(); eit != dt.finite_edges_end(); ++eit)
  {
    const DT::Face_handle& fh = eit->first;
    if (dt.is_infinite(fh) || dt.is_infinite(fh->neighbor(eit->second)))
    {      // Border edge
      segments.push_back(dt.segment(eit));
      std::cout << "segments p1 = " <<dt.segment(eit).source()
                << " segments p2 = " <<dt.segment(eit).target()<< std::endl;

    }
  }
  int counter = 0;
  hulls->push_back(segments.begin()->source());
  hulls->push_back(segments.begin()->target());
  Point first_p = segments.front().target();
  segments.erase(segments.begin());
  Point second_p;
  do{
    std::cout << "first p =  " <<first_p;

    if(getNextBorderPoint(&segments,first_p ,&second_p))
    {
      std::cout << " next p =  " <<second_p<< std::endl;
      hulls->push_back(second_p);
    } else {
      std::cout << "ERROR " << std::endl;
    }
    first_p = second_p;
    counter++;
  } while(counter<100 && segments.size()>0);
  std::cout << "counter " <<counter<< std::endl;

}

void makeHullold(const PointVector &points,
                 PointVector *hulls)
{
  PointVector hull;
  CGAL::convex_hull_2( points.begin(), points.end(), std::back_inserter( hull ) );
  DT dt;
  int nhullvert = hull.size();
  std::vector<Vertex_handle> vvconstraint(nhullvert);
  dt.insert( points.begin(),points.end() );

  for(int i = 0; i < nhullvert; ++i){
    vvconstraint[i] = dt.insert(hull.at(i));
  }

  for(int i=0; i < nhullvert; ++i){
    Vertex_handle p1 = vvconstraint[i];
    Vertex_handle p3 = vvconstraint[(i+1)%nhullvert];
    Vertex_handle p2;
    if(!getAddPoint(p1,p3,dt,&p2)){
      hulls->push_back(p1->point());
      hulls->push_back(p3->point());
    }
    else {
      hulls->push_back(p1->point());
      hulls->push_back(p2->point());
      hulls->push_back(p3->point());
    }
    //)<<p1->point()<<p3->point()<<p2->point();
  }

}

QVector< QPolygon> CgalIso::alpha(const QPolygon & xy, int al )
{
  int n = xy.size();
  std::vector< Point > points;
  for(int i = 0; i < n; ++i){
    points.push_back(Point(xy.at(i).x()/6371.,xy.at(i).y()/6371.));
  }

  Alpha_shape_2 A(points.begin(), points.end(),
                  Alpha_shape_2::REGULARIZED);
  A.set_alpha(al);

  std::cout << "Alpha Shape number_of_alphas<"<<A.number_of_alphas() << std::endl;
  std::cout << "Alpha Shape number_of_alphas<"<<A.number_of_alphas() << std::endl;
  std::cout << "Optimal alpha: " << *A.find_optimal_alpha(1) << std::endl;
  QVector< QPolygon> trret;

  for (Alpha_shape_2::Finite_faces_iterator fit=A.finite_faces_begin();fit!=A.finite_faces_end();++fit){
    if (A.classify(fit)==Alpha_shape_2::INTERIOR){
      QPolygon triangl;
      AVertex_handle opposite1 = fit->vertex(0);
      QPoint p1 = QPoint(opposite1->point().hx()*6371.,opposite1->point().hy()*6371.);
      triangl<<p1;
      opposite1 = fit->vertex(1);
      p1 = QPoint(opposite1->point().hx()*6371.,opposite1->point().hy()*6371.);
      triangl<<p1;
      opposite1 = fit->vertex(2);
      p1 = QPoint(opposite1->point().hx()*6371.,opposite1->point().hy()*6371.);
      triangl<<p1;
      trret<<triangl;
    }
  }
  return trret;
}


void makeAlpha(const PointVector &points,
               PointVector *hulls,int alpha){

  Alpha_shape_2 A(points.begin(), points.end(),
                  FT(alpha),
                  Alpha_shape_2::GENERAL);
  std::vector<Segment> segments;



  for(Alpha_shape_edges_iterator it =  A.alpha_shape_edges_begin();
      it != A.alpha_shape_edges_end();
      ++it){
    hulls->push_back(A.segment(*it).source());
    hulls->push_back(A.segment(*it).target());
  }

}


QMap<int, QPolygon> CgalIso::triang(const QPolygon & xy,
                                    const QVector<meteo::GeoPoint>& gp,
                                    QVector< QPolygon> * ,
                                    uint min_count,
                                    float max_rast,
                                    int )
{
  int n = xy.size();
  DT dt;
  std::map<Vertex_handle, meteo::GeoPoint> dmap;
  for(int i = 0; i < n; ++i){
    const Point& point = Point(xy.at(i).x(),xy.at(i).y());
    //points.push_back(std::make_pair(Point(xy.at(i).x(),xy.at(i).y()),fun[i]));
    Vertex_handle avertex = dt.insert( point);
    avertex->info() = -1;
    dmap.insert(std::make_pair(avertex,gp.at(i)));
  }
  int num = 0;
  dt.finite_vertices_begin()->info()=num;
  std::map<int, PointVector> clusters_;
  std::map<int, PointVector> hulls_;
  std::map<std::pair<Point,Point>, int> dist_matrix;

  hulls_.clear();
  clusters_.clear();
  for( DT::Finite_vertices_iterator fit = dt.finite_vertices_begin();
       fit != dt.finite_vertices_end(); ++fit )
  {
    Vertex_handle vertex = fit;
    if(vertex->info() > -1) continue;
    vertex->info() = num;
    PointVector points;
    std::vector<PointVector> hull1;
    std::vector<PointVector> hull;
    std::list<Polygon_with_holes_2> hole;
    getNears(&dt,vertex,dmap,num,&points,max_rast,&dist_matrix);
    if(points.size() >= min_count){
      clusters_.insert(std::make_pair(num,points));
      //CGAL::convex_hull_2( points.begin(), points.end(), std::back_inserter( hull ) );
      //*triang+=getTriang(DT(points.begin(), points.end()));

      //makeAlpha(points,&hull,alpha);
      //makeHull(points,&hull);
      QTime tm = QTime::currentTime();
     // qDebug()<<"makeJoinCircle start"<<QTime::currentTime().toString("HH:mm:ss.zzz");
     // makeJoinCircle(points,&hull1);

     // qDebug()<<"TOCHN dur "<<tm.msecsTo(QTime::currentTime());

      tm = QTime::currentTime();
      makeJoinCircle1(points,&hull);
      qDebug()<<"CURVE dur "<<tm.msecsTo(QTime::currentTime());




      // Print the result.
      /*  std::copy (hole.begin(), hole.end(),
                 std::ostream_iterator<Polygon_with_holes_2>(std::cout, "\n"));
      std::cout << std::endl;
*/
      std::vector<PointVector>::iterator hit = hull.begin();
      std::vector<PointVector>::iterator ehit = hull.end();
      for(;hit != ehit; ++hit)
        hulls_.insert(std::make_pair(num++,*hit));
    }
    ++num;
  }

  /* QMap<int, QPolygon> trret;
  for( DT::Finite_vertices_iterator fit = dt.finite_vertices_begin();
       fit != dt.finite_vertices_end(); ++fit )
  {
    Vertex_handle vertex = fit;
    QPoint p1 = QPoint(vertex->point().hx()*6371.,vertex->point().hy()*6371.);
    trret[vertex->info()]<<p1;
  }*/

  QMap<int, QPolygon> trret;
  for( std::map<int, PointVector>::iterator fit = hulls_.begin();
       fit != hulls_.end(); ++fit )
  {
    PointVector p = (*fit).second;
    int key = (*fit).first;
    QPolygon pol;
    for( PointVector::iterator pit = p.begin();
         pit != p.end(); ++pit )
    {
      QPoint p1 = QPoint(pit->x(),pit->y());
      pol<<p1;
    }
    trret[key]<<pol;
  }


  return trret;
}

QVector< QPolygon> CgalIso::triangOnly(const QPolygon & xy, const QVector<float>& fun)
{
  int n = xy.size();
  std::vector< std::pair<Point,unsigned> > points;
  for(int i = 0; i < n; ++i){
    points.push_back(std::make_pair(Point(xy.at(i).x(),xy.at(i).y()),fun[i]));
  }
  DT dt;
  dt.insert( points.begin(),points.end() );
  return getTriang(dt);
}




/*QList<GeoPolygon*> list = layer_->objectsByType<GeoPolygon*>(false);

  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    Object* o = list[i];
    delete o;
  }

  CgalIso cgal;
  warning_log << "start triangulate 3d - ";

  QPolygon scelet;
  QVector<float> data;
  bool ok;
  //QVector<QVector3D> sphere;
  QVector<GeoPoint> gd;
 float probability = ui_->probability->text().toFloat();
  float min_points = ui_->min_points->text().toFloat();
  float epsilon = ui_->epsilon->text().toFloat();
  float cluster_epsilon = ui_->cluster_epsilon->text().toFloat();
  float normal_threshold = ui_->normal_threshold->text().toFloat();
  cgal.setParam(probability,min_points,epsilon,cluster_epsilon,normal_threshold);

  for ( Puanson* o : l->objectsByType<Puanson*>() ) {
    const QVector<QPolygon>& cp = o->cartesianSkelet();

    const TMeteoParam &mp = o->paramValue("sss",&ok);
    if( mp.value() ==0||  mp.isInvalid() || control::QualityControl::NO_CONTROL < mp.quality()){
      continue;
    }
    gd << o->skelet().at(0);
    data.append(mp.value());
    scelet.append(cp.at(0).at(0));
  }

  int min_points = ui_->min_points->text().toInt();

  int epsilon = ui_->epsilon->text().toInt();
  int cluster_epsilon = ui_->cluster_epsilon->text().toInt();
  QVector< QPolygon> vgv;
  QMap<int, QPolygon> ggv = cgal.triang(scelet,gd,&vgv
                                        ,min_points,cluster_epsilon,epsilon);
  //QVector< QPolygon> ggv = cgal.triang(scelet,data);
  warning_log << "stop triangulate 3d" <<ggv.size();




  for(;beg != end; ++beg){
    QPolygon pol = beg.value();
    int key = beg.key();
    for(int i =0; i < pol.size();++i){
      GeoText* label = new GeoText(layer_);
      QPen pen = label->qpen();
      QFont font = label->qfont();
      pen.setColor(QColor(65,45,20));
      font.setPointSize(20);
      label->setPen(pen);
      label->setFont(font);
      label->setPos(kCenter);
      label->setDrawAlways(true);
      label->setText(QString::number(key));
      label->setCartesianSkelet(QPolygon()<< pol.at(i));
    }
  }



  QVector< QPolygon> ggv = cgal.alpha(scelet, min_points);



 for(int i =0; i < vgv.size();++i){
    GeoPolygon* gline = new GeoPolygon(layer_ );
    gline->setPenStyle(Qt::SolidLine);
    gline->setPenWidth(2);
    gline->setPenColor(Qt::blue);
    gline->setSplineFactor(1);
    gline->setClosed(true);
  //gline->setSkelet( ggv.at(i) );
    gline->setCartesianSkelet(vgv.at(i));
  }
 QMap<int, QPolygon>::iterator beg = ggv.begin();
 QMap<int, QPolygon>::iterator end = ggv.end();

 for(;beg != end; ++beg){
   QPolygon pol = beg.value();
   //int key = beg.key();
     GeoPolygon* gline = new GeoPolygon(layer_ );
     gline->setPenStyle(Qt::SolidLine);
     gline->setPenWidth(2);
     gline->setPenColor(Qt::red);
     gline->setSplineFactor(1);
     //gline->setClosed(true);
     //gline->setSkelet( ggv.at(i) );
     gline->setCartesianSkelet(pol);
 }

  warning_log << "stop triangulate";*/
