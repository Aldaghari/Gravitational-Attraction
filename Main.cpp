#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

float constrain(float value, float min, float max)
{
    if (value < min)
        return min;
    else if (value > max)
        return max;
    else
        return value;
}

class Track
{
private:
    // Related to addTrack method.
    sf::Image trackImage;
    sf::Texture trackTexture;
    sf::Sprite shape;

public:
    Track(const sf::Color& color = sf::Color::White)
    {
        // related to addTrack method.
        trackImage.create(1, 1, color);
        trackTexture.loadFromImage(trackImage);
        shape.setTexture(trackTexture);
    }

    sf::Sprite getShape() const { return shape; }

};

class Planet
{
private:
    sf::Vector2f pos;
    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    float radius;
    float mass;
    const float gravitationalConstant;
    sf::Color color;
    sf::Color outlineColor;
    float outlineThickness;
    sf::CircleShape shape;

    // Related to spawnAtMousePos method.
    bool stickToCursor = false;

    // Related to addTrack method.
    std::vector<sf::Sprite> track;
    bool haveTrack = false;

    // Related to select method.
    bool isSelected = false;

public:
    Planet(float r, float m, float g, const sf::Color& c) : pos(0.0f, 0.0f), velocity(0.0f, 0.0f), acceleration(0.0f, 0.0f),
        radius{ r }, mass{ m }, gravitationalConstant{ g }, color{ c }, outlineColor{ sf::Color::Red }, outlineThickness{ 0.0f }
    {
        shape.setOrigin(radius, radius);
        shape.setPosition(pos);
        shape.setRadius(radius);
        shape.setFillColor(color);
        shape.setOutlineColor(outlineColor);
        shape.setOutlineThickness(outlineThickness);
    }

    void setPos(float x, float y)
    {
        pos.x = x;
        pos.y = y;
    }

    void setColor(sf::Color c)
    {
        shape.setFillColor(c);
    }

    void update()
    {
        velocity += acceleration;
        pos += velocity;
        shape.setPosition(pos);

        // Reset accleration
        acceleration = sf::Vector2f(0.0f, 0.0f);

        //std::cout << "x: " << pos.x << " " << "y: " << pos.y << std::endl;
    }

    void draw(sf::RenderWindow& window)
    {
        window.draw(shape);

        // Draw track if available.
        if (haveTrack)
        {
            for (int i = 0; i < track.size(); i++)
            {
                window.draw(track[i]);
            }
        }
    }

    void applyGravityForce(const Planet& otherPlanet)
    {
        if (!stickToCursor)
        {
            sf::Vector2f distance = otherPlanet.pos - this->pos;
            float distanceMag = sqrt(distance.x * distance.x + distance.y * distance.y);
            sf::Vector2f distanceNorm = distance / distanceMag;

            distanceMag = constrain(distanceMag, 5, 25);

            // f = ( G * m1 * m2 * distanceNorm) / distance^2
            // Multiplying:
            sf::Vector2f force = otherPlanet.gravitationalConstant * this->mass * otherPlanet.mass * distanceNorm;
            // Division:
            force = force / (distanceMag * distanceMag);
            acceleration += force / mass;   
        }
    }

    void setVelocity(const sf::Vector2f& amount)
    {
        if (!stickToCursor)
            velocity = amount;
    }

    void select(const sf::Event& event, const sf::RenderWindow& window)
    {
        if (event.type == sf::Event::MouseButtonPressed)
        {
            sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));
            sf::Vector2f distance = mousePos - pos;
            float magOfDistance = sqrt(distance.x * distance.x + distance.y * distance.y);

            if (event.mouseButton.button == sf::Mouse::Right && magOfDistance < radius)
            {
                if (!isSelected)
                {
                    outlineThickness = 2.0f;
                    shape.setOutlineThickness(outlineThickness);
                    isSelected = true;
                }
                else
                {
                    outlineThickness = 0.0f;
                    shape.setOutlineThickness(outlineThickness);
                    isSelected = false;
                }
            }
        }
    }

    void drag(const sf::Event& event, const sf::RenderWindow& window)
    {
        if (event.type == sf::Event::MouseButtonPressed)
        {
            sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));
            sf::Vector2f distance = mousePos - pos;
            float magOfDistance = sqrt(distance.x * distance.x + distance.y * distance.y);

            if (event.mouseButton.button == sf::Mouse::Left && magOfDistance < radius)
            {
                pos = mousePos;
                acceleration *= 0.0f;
                velocity *= 0.0f;
                stickToCursor = true;
            }
        }
        else if (stickToCursor)
        {
            pos = sf::Vector2f(sf::Mouse::getPosition(window));
            acceleration *= 0.0f;
            velocity *= 0.0f;
        }

        if (event.type == sf::Event::MouseButtonReleased)
        {
            stickToCursor = false;
        }
    }

    void addTrack(Track& t)
    {
        // If held by mouse don't add track.
        if (!stickToCursor)
        {
            sf::Sprite shapeOfTrack = t.getShape();
            shapeOfTrack.setPosition(pos);
            track.push_back(shapeOfTrack);
            haveTrack = true;
        }
    }

    sf::CircleShape getShape() const { return shape; }
};


int main()
{
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    sf::RenderWindow window(sf::VideoMode(1920, 1080), "My window", sf::Style::Fullscreen, settings);
    //window.setFramerateLimit(30);

    // Setup

    // Sun
    Planet sun(60.0f, 40.0f, 0.1f, sf::Color(253, 184, 19));
    sun.setPos(window.getSize().x / 2, window.getSize().y / 2);
    
    // Planet
    Track PlanetTrack(sf::Color(122, 76, 26));

    Planet planet(25.0f, 10.0f, 0.01f, sf::Color::Magenta);
    planet.setPos(window.getSize().x / 2 - 300, window.getSize().y / 2 + 200);
    planet.setVelocity(sf::Vector2f(-2.0f, 0.0f));

    // Earth
    Track earthTrack(sf::Color(164, 25, 61));

    Planet earth(25.0f, 10.0f, 0.01f, sf::Color::Cyan);
    earth.setPos(window.getSize().x / 2 + 300, window.getSize().y / 2 - 200);
    earth.setVelocity(sf::Vector2f(2.0f, 0.0f));


    while (window.isOpen())
    {
        sf::Event event;
        sf::Mouse mouse;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed || event.key.code == sf::Keyboard::Escape)
                window.close();


            earth.drag(event, window);
            earth.select(event, window);

            planet.drag(event, window);
            planet.select(event, window);
        }

        // Update
        sun.update();

        earth.applyGravityForce(planet);
        earth.applyGravityForce(sun);
        earth.update();
        earth.addTrack(earthTrack);

        planet.applyGravityForce(earth);
        planet.applyGravityForce(sun);
        planet.update();
        planet.addTrack(PlanetTrack);


        // Draw
        window.clear(sf::Color::White);
        sun.draw(window);
        earth.draw(window);
        planet.draw(window);
        window.display();

    }

    return 0;
}