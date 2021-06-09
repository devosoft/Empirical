#ifndef SFML_LINESHAPE_HPP
#define SFML_LINESHAPE_HPP

#include <cmath>

#include <SFML/Graphics/Export.hpp>
#include <SFML/Graphics/Shape.hpp>

// adapted from https://github.com/SFML/SFML/wiki/Source:-Line-segment-with-thickness

namespace sf {

class SFML_GRAPHICS_API LineShape : public Shape {
	public:

	explicit LineShape(const Vector2f& point1, const Vector2f& point2);

	void setThickness(float thickness);

	float getThickness() const;

	float getLength() const;

	virtual size_t getPointCount() const;

	virtual Vector2f getPoint(size_t index) const;

	private:

  Vector2f m_direction; ///< Direction of the line
  float m_thickness;    ///< Thickness of the line
};


LineShape::LineShape(const Vector2f& point1, const Vector2f& point2)
: m_direction(point2 - point1) {
    setPosition(point1);
    setThickness(2.f);
}


void LineShape::setThickness(const float thickness) {
    m_thickness = thickness;
    update();
}


float LineShape::getThickness() const {
    return m_thickness;
}


float LineShape::getLength() const {
    return std::sqrt(m_direction.x*m_direction.x+m_direction.y*m_direction.y);
}


size_t LineShape::getPointCount() const {
    return 4;
}

Vector2f LineShape::getPoint(const size_t index) const {
    Vector2f unitDirection = m_direction/getLength();
    Vector2f unitPerpendicular(-unitDirection.y,unitDirection.x);

    Vector2f offset = (m_thickness/2.f)*unitPerpendicular;

    switch (index) {
        default:
        case 0: return offset;
        case 1: return (m_direction + offset);
        case 2: return (m_direction - offset);
        case 3: return (-offset);
    }
}

} // namespace sf


#endif // SFML_LINESHAPE_HPP
