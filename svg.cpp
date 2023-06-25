#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream& operator<< (std::ostream& out, const svg::StrokeLineCap& line_cap) {
    static const std::vector<std::string_view> string_caps_ = {"butt"sv, "round"sv, "square"sv};
    out << string_caps_.at(static_cast<int>(line_cap));
    return out;
} 
    
std::ostream& operator<< (std::ostream& out, const svg::StrokeLineJoin& line_join) {
    static const std::vector<std::string_view> line_join_caps_ = {"arcs"sv, "bevel"sv, "miter"sv, "miter-clip"sv, "round"sv};
    out << line_join_caps_.at(static_cast<int>(line_join));
    return out;
}


void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Class Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Class Polyline ------------------  
    
// Add Point into Polyline
Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(std::move(point));
    return *this;
}
    
// Render Polyline into output stream
void Polyline::RenderObject(const RenderContext& context) const {
    auto& out =  context.out;
    out << "<polyline points=\""sv;
    bool is_first = true;
    for (const auto& point : points_) {
        if (!is_first) {
            out << " ";
        }
        is_first = false;
        out << point.x << ","sv << point.y;
    }
    out << "\""s;
    RenderAttrs(out);
    out << " />"sv;
}
    

 // ----------  Class Text ------------------    

// Set text content (inside tag "text" )
Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}
    
// Set font thickness (attribute font-weight)
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}
    
// Set font name (attribute font-family)
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}
    
 // Set font size (attribute font-size)
Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}
    
// Set offset relatively of reference point (attribute dx, dy)
Text& Text::SetOffset(Point offset) {
    offset_ = std::move(offset);
    return *this;
}
    
// Set coordinates of reference point (attribute x и y)
Text& Text::SetPosition(Point pos) {
    position_ = std::move(pos);
    return *this;
}
    
    
void Text::RenderObject(const RenderContext& context) const {
    auto& out  = context.out;
    out << "<text x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" "sv;
    out << "dx=\"" << offset_.x << "\" dy=\""sv << offset_.y << "\" font-size=\""sv << font_size_ << "\""sv;

    if (!font_family_.empty()) {
        out << " font-family=\""sv << font_family_ << "\""sv;
    }
    
    if (!font_weight_.empty()) {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }
    RenderAttrs(out);
    out <<">"sv << data_ << "</text>"sv;
}

// ----------  class Document ------------------   
    
// Add unique_ptr
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    documents_.emplace_back(std::move(obj));
} 

// Get and Add offspring of Object
   
// Renders all objects into out-stream using Object::Render and Fig::RenderObject
void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for (const auto &object : documents_) {
        object->Render(RenderContext(out));
    }
    out << "</svg>"sv;
}

}  // namespace svg