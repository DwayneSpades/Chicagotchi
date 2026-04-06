struct rect {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    int maxX() const { return x + w; }
    int maxY() const { return y + h; }

    bool intersects(const rect& other) {
        return ((x < other.maxX() || maxX() > other.x) && (y < other.maxY() || y > other.maxY()));
    }
};
