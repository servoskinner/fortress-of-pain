#include <SFML/Graphics.hpp>

int main()
{
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Corporate Wars");
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    while (window.isOpen())
    {
        sf::Event event;


        //icon:
        sf::Image icon;
        if (!icon.loadFromFile("Source/images/icon.png")) {
            return 1;
        }
        window.setIcon(976, 976, icon.getPixelsPtr());



        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(shape);
        window.display();
    }

    return 0;
}