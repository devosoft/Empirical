#ifndef EMP_LINE_2D_H
#define EMP_LINE_2D_H

#include "Point2D.h"

namespace emp {

    template <typename TYPE>
    class Rect2D;
    template <typename TYPE>
    class Circle2D;

    template <typename TYPE=double> class Line2D {
    private:
        Point2D<TYPE> p1;
        Point2D<TYPE> p2;

    public:
        Line2D<TYPE>(Point2D<TYPE> p1, Point2D<TYPE> p2) : p1(p1), p2(p2) {}
        Line2D<TYPE>() : p1(0,0), p2(0,0) {}

        constexpr inline Point2D<TYPE> GetP1() const {return p1;}
        constexpr inline Point2D<TYPE> GetP2() const {return p2;}

        void SetP1(Point2D<TYPE> new_p1) {p1 = new_p1;}
        void SetP2(Point2D<TYPE> new_p2) {p2 = new_p2;}


        TYPE GetLength(){return p1.Distance(p2);}
        bool IsOn(const Point & q) const {
            // From http://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
            if (q.GetX() <= std::max(p1.GetX(), p2.GetX()) && q.GetX() >= std::min(p1.GetX(), p2.GetX()) &&
                q.GetY() <= std::max(p1.GetY(), p2.GetY()) && q.GetY() >= std::min(p1.GetY(), p2.GetY()))
               return true;

            return false;
        }

        // To find orientation of ordered triplet (p, q, r).
        // The function returns following values
        // 0 --> p, q and r are colinear
        // 1 --> Clockwise
        // 2 --> Counterclockwise
        int orientation(const Point & p, const Point & q, const Point & r) {
            // From http://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
            // See http://www.geeksforgeeks.org/orientation-3-ordered-points/
            // for details of below formula.
            int val = (q.GetY() - p.GetY()) * (r.GetX() - q.GetX()) -
                      (q.GetX() - p.GetX()) * (r.GetY() - q.GetY());

            if (val == 0) return 0;  // colinear

            return (val > 0)? 1: 2; // clock or counterclock wise
        }

        bool Intersects(const Line2D & _in) {
            // From http://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
            // Find the four orientations needed for general and
            // special cases
            int o1 = orientation(p1, p2, _in.GetP1());
            int o2 = orientation(p1, p2, _in.GetP2());
            int o3 = orientation(_in.GetP1(), _in.GetP2(), p1);
            int o4 = orientation(_in.GetP1(), _in.GetP2(), p2);

            // General case
            if (o1 != o2 && o3 != o4)
                return true;

            // Special Cases
            // p1, q1 and p2 are colinear and p2 lies on segment p1q1
            if (o1 == 0 && IsOn(_in.GetP1())) return true;

            // p1, q1 and p2 are colinear and q2 lies on segment p1q1
            if (o2 == 0 && IsOn(_in.GetP2())) return true;

            // p2, q2 and p1 are colinear and p1 lies on segment p2q2
            if (o3 == 0 && _in.IsOn(p1)) return true;

             // p2, q2 and q1 are colinear and q1 lies on segment p2q2
            if (o4 == 0 && _in.IsOn(p2)) return true;

            return false; // Doesn't fall in any of the above cases
        }

        // template <typename TYPE2>
        // bool Intersects(const Circle2D<TYPE2> & circ) {
        //
        // }
        template <typename TYPE2>
        bool Intersects(const Rect2D<TYPE2> & rect) {
            // @ELD TODO: I'm positive this could be way more efficient
            return Intersects(rect.GetTop()) || Intersects(rect.GetBottom())
                    || Intersects(rect.GetLeft()) || Intersects(rect.GetRight());
        }

    };

    using Line = Line2D<>;

}

#endif
