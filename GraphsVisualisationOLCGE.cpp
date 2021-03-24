#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"//https://github.com/OneLoneCoder/olcPixelGameEngine
#include "Real_World_Algorithms.h"//https://github.com/ikvasir/Real_World_Algorithms
#include <algorithm>
#include <numbers>
#include <chrono>
#include <thread>
#include <future>

#define SCREEN_SIZE_X 1000
#define SCREEN_SIZE_Y 1000
#define CIRCLE_RADIUS 30
#define BUTTON_HEIGHT int32_t(50)
#define BUTTON_WIDTH int32_t(400)

//number of squears horisontally and vertically in labirinth task
//change this like 100,100 or 10,10 or 5,10 
#define FIELD_SIZE_X int32_t(25)
#define FIELD_SIZE_Y int32_t(25)

//delay in labirinth drawing in ms
#define DRAW_DELAY 10


class GraphsVisualisation : public olc::PixelGameEngine
{
	enum class task{welcome_screen, graph_visualisation, labirinth_visualisation};
	enum class labirinth_task {NONE,DFS_recurcive, DFS_stack, BFS, Random,Clear};

	task task_selected = task::welcome_screen;
	labirinth_task ltask_selected = labirinth_task::NONE;

	graph<int, FIELD_SIZE_X * FIELD_SIZE_Y> labirint_graph;

	struct GraphVertex
	{
		GraphVertex(olc::vf2d pos_, int32_t radius_, std::string sname_,int iname_):pos(pos_),radius(radius_),sname(sname_),iname(iname_) {};
		olc::vf2d pos;
		int32_t radius;
		std::string sname;
		int iname;
	};
	
	struct GraphEdge
	{
		GraphEdge(int name1, olc::vf2d pos1, int name2, olc::vf2d pos2) 
			{
			pos.first = pos1;
			pos.second = pos2;
			name.first = name1;
			name.second = name2;
			};
		bool directed_graph = false;
		std::pair<olc::vf2d, olc::vf2d> pos;
		std::pair<int, int> name;
	};

	std::vector<GraphVertex> vertexes_;
	std::vector<GraphEdge> edges_;

	GraphVertex* pSelectedVertex = nullptr;

	std::vector<GraphVertex> generateRandomVertexesPos(auto graph) {
		std::vector<GraphVertex> vrtx;

		for (auto& g : graph.V) {
			vrtx.push_back(GraphVertex(olc::vi2d{ BUTTON_WIDTH+RAND(CIRCLE_RADIUS / 2, SCREEN_SIZE_X - CIRCLE_RADIUS / 2),
									    RAND(CIRCLE_RADIUS / 2, SCREEN_SIZE_Y - CIRCLE_RADIUS / 2) },
										CIRCLE_RADIUS,
										std::to_string(g),
										g));
		}
		return std::move(vrtx);
	};

	void UpdateEdgesPosition() {
		float PI = std::numbers::pi_v<float>;
		for (auto& e : edges_) {
			auto iter1 = std::find_if(vertexes_.begin(), vertexes_.end(), [&](GraphVertex gv) {return (gv.iname == e.name.first); });
			auto iter2 = std::find_if(vertexes_.begin(), vertexes_.end(), [&](GraphVertex gv) {return (gv.iname == e.name.second); });

			float Angle1 = atan2f(iter1->pos.y - iter2->pos.y, iter1->pos.x - iter2->pos.x);
			float Angle2 = atan2f(iter2->pos.y - iter1->pos.y, iter2->pos.x - iter1->pos.x);

			olc::vf2d pos1 = olc::vf2d{ iter1->pos.x + CIRCLE_RADIUS * cos(Angle1 + PI), iter1->pos.y + CIRCLE_RADIUS * sin(Angle1 + PI) };
			olc::vf2d pos2 = olc::vf2d{ iter2->pos.x + CIRCLE_RADIUS * cos(Angle2 + PI), iter2->pos.y + CIRCLE_RADIUS * sin(Angle2 + PI) };

			e.pos.first = pos1;
			e.pos.second = pos2;

			
		}
	};

	std::vector<GraphEdge> generateEdgesPos(std::vector<GraphVertex> vrtx, auto graph) {
		
		float PI = std::numbers::pi_v<float>;
		std::vector<GraphEdge> edgs;
		for (auto& e : graph.E) 
		{
			auto iter1 = std::find_if(vrtx.begin(), vrtx.end(), [&](GraphVertex gv) {return (gv.iname == e.first); });
			auto iter2 = std::find_if(vrtx.begin(), vrtx.end(), [&](GraphVertex gv) {return (gv.iname == e.second); });
			
			float Angle1 = atan2f(iter1->pos.y - iter2->pos.y, iter1->pos.x- iter2->pos.x);
			float Angle2 = atan2f(iter2->pos.y - iter1->pos.y, iter2->pos.x - iter1->pos.x);
			olc::vf2d pos1 = olc::vf2d{ iter1->pos.x + CIRCLE_RADIUS * cos(Angle1 + PI), iter1->pos.y + CIRCLE_RADIUS * sin(Angle1 + PI) };
			olc::vf2d pos2 = olc::vf2d{ iter2->pos.x + CIRCLE_RADIUS * cos(Angle2 + PI), iter2->pos.y + CIRCLE_RADIUS * sin(Angle2 + PI) };

			edgs.push_back(GraphEdge(e.first,pos1, e.second,pos2));
		}
		return std::move(edgs);
	};

	void mouseControlling() {

		auto isPointOverlap = [](const int32_t& x, const int32_t& y, GraphVertex& vertex) {
			return x < (vertex.pos.x + vertex.radius) &&
				x >(vertex.pos.x - vertex.radius) &&
				y > (vertex.pos.y - vertex.radius) &&
				y < (vertex.pos.y + vertex.radius);
		};

		auto isPointOverlapRect = [](const int32_t& x, const int32_t& y,
			const int32_t& rX, const int32_t& rY,
			const int32_t& rW, const int32_t& rH) {

				return x < (rX + rW) &&
					x >(rX) &&
					y < (rY + rH) &&
					y >(rY);
		};

		if (GetMouse(0).bPressed || GetMouse(1).bPressed)
		{
				for(auto& b : buttons_[static_cast<int32_t>(task_selected)])
					if(isPointOverlapRect(GetMouseX(), GetMouseY(), b.possition.x, b.possition.y, b.size.x, b.size.y))
				{
						if (b.text == "RandomGraph") {
							task_selected = task::graph_visualisation;

							graph<int, 10> graph_ = generateRandomGraph<int, 10>();
							graph_.DFS_recurcive(0);
							vertexes_ = generateRandomVertexesPos(graph_);
							edges_ = generateEdgesPos(vertexes_, graph_);

						}
						else if (b.text == "2d labirinth creation") {
							task_selected = task::labirinth_visualisation;
							labirint_graph = generateLabirinthGraph<int, FIELD_SIZE_X, FIELD_SIZE_Y>();
						}
						else if (b.text == "Menu")task_selected = task::welcome_screen;
						else if (b.text == "DFS_recurcive") ltask_selected = labirinth_task::DFS_recurcive;
						else if (b.text == "DFS_stack")ltask_selected = labirinth_task::DFS_stack;
						else if (b.text == "BFS")ltask_selected = labirinth_task::BFS;
						else if (b.text == "Random")ltask_selected = labirinth_task::Random;
						else if (b.text == "Clear")ltask_selected = labirinth_task::Clear;
						else if (b.text == "EXIT")exit(1);
						

					std::cout << "rect call:" << b.text << std::endl;
				};

			
			if (task_selected == task::graph_visualisation)
			{
				pSelectedVertex = nullptr;

				for (auto& v : vertexes_)
					if (isPointOverlap(GetMouseX(), GetMouseY(), v))
					{
						pSelectedVertex = &v;
						break;
					}
			}
		}

		if (GetMouse(0).bHeld)
		{

			if (pSelectedVertex != nullptr)
			{
				pSelectedVertex->pos.x = GetMouseX();
				pSelectedVertex->pos.y = GetMouseY();
				UpdateEdgesPosition();
			}
		}
	};

	///BUTTONS
	struct Button {
		olc::vi2d possition;
		olc::vi2d size;
		std::string text;
		olc::Pixel color=olc::WHITE;
	};
	using vec_but = typename std::vector<Button>;
	std::vector<vec_but> buttons_;
	int32_t current_button_pos[10]; 


	void createButton(olc::vi2d possition, olc::vi2d size, std::string text) {
		buttons_[static_cast<int32_t>(task_selected)].push_back(Button(possition, size, text));
	}

	void drawButton(Button btn) {
		DrawRect(btn.possition, btn.size, btn.color);
		DrawString(btn.possition + olc::vi2d{ BUTTON_HEIGHT / 2, BUTTON_HEIGHT / 2}, btn.text, btn.color,2);
	}

	void drawButton(std::string text) {
		//for (auto& p : current_button_pos)std::cout << p << " "; std::cout << std::endl;
		if (buttons_.empty())buttons_.resize(10);

		auto iter = std::find_if(buttons_[static_cast<int32_t>(task_selected)].begin(), buttons_[static_cast<int32_t>(task_selected)].end(), [&](Button btn) {return text == btn.text; });
		if (iter == buttons_[static_cast<int32_t>(task_selected)].end()) {
			
			createButton(olc::vi2d{0, BUTTON_HEIGHT + current_button_pos[static_cast<int32_t>(task_selected)] }, olc::vi2d{ BUTTON_WIDTH, BUTTON_HEIGHT }, text);
			current_button_pos[static_cast<int32_t>(task_selected)] += BUTTON_HEIGHT+BUTTON_HEIGHT/2;
		}
		else

		{
			drawButton(*iter);
		}
		
	};

	///  graph

	void drawGraph_1() {
		
		for (auto& e : edges_)
		{
			DrawLine(e.pos.first, e.pos.second);
		}
		for (auto& v : vertexes_) {
			DrawCircle(v.pos, v.radius);
			DrawString(v.pos - olc::vi2d{ CIRCLE_RADIUS / 2,CIRCLE_RADIUS / 3 }, v.sname, olc::WHITE, 3);
		}
	};

	/// labirinth

	struct labirinth_cell {
		olc::vi2d pos;
		olc::vi2d size;
		olc::Pixel color;
	};
	
	std::vector<labirinth_cell> labirinth_;

	void createLabirinth(int32_t X, int32_t Y) {

		int32_t rect_width = SCREEN_SIZE_X / FIELD_SIZE_X;
		int32_t rect_height = SCREEN_SIZE_Y / FIELD_SIZE_Y;
		labirinth_.resize(X * Y);

		for (int32_t i = 0; i < FIELD_SIZE_X; i++)
			for (int32_t j = 0; j < FIELD_SIZE_Y; j++)
			{
				labirinth_[i*Y+j]=labirinth_cell(olc::vi2d{ BUTTON_WIDTH + i * rect_width, j * rect_height },
													olc::vi2d{ rect_width, rect_height },
													olc::WHITE);
			}
	};

	void drawLabirinthField() {
		for (auto& r : labirinth_)FillRect(r.pos, r.size, r.color);

	};

	void drawLabirinthNet() {
		for (int32_t i = 0; i < FIELD_SIZE_X; i++)
			DrawLine(BUTTON_WIDTH + i * SCREEN_SIZE_X / FIELD_SIZE_X, 0, BUTTON_WIDTH + i * SCREEN_SIZE_X / FIELD_SIZE_X, SCREEN_SIZE_Y,olc::RED);

		for (int32_t i = 0; i < FIELD_SIZE_Y; i++)
			DrawLine(BUTTON_WIDTH, i * SCREEN_SIZE_Y / FIELD_SIZE_Y, BUTTON_WIDTH+SCREEN_SIZE_Y, i*SCREEN_SIZE_Y/FIELD_SIZE_Y, olc::RED);

	};

	std::function<void()> visualisation_func_update = [&]() {
		auto xy = labirint_graph.getLastVisitedNode();
		labirinth_[xy].color = olc::GREEN;
		std::this_thread::sleep_for(std::chrono::milliseconds(DRAW_DELAY));
	};

	 uint32_t  visualisation_func_update_color ;
	 olc::Pixel good_random_color=olc::GREEN;

	 std::function<void()> visualisation_func_update_colors = [&]() {
		 auto xy = labirint_graph.getLastVisitedNode();

		 if (visualisation_func_update_color != labirint_graph.getNumberCurrentPath()) {
			 visualisation_func_update_color = labirint_graph.getNumberCurrentPath();
			 uint8_t r = RAND(0, 255) ;
			 uint8_t g = RAND(0, 255) ;
			 uint8_t b = RAND(0, 255) ;
			 good_random_color = olc::Pixel(r,g,b);
		 }

		labirinth_[xy].color = good_random_color;

		std::this_thread::sleep_for(std::chrono::milliseconds(DRAW_DELAY));
	 };

	void drawWelcomeScreen() {
		drawButton("RandomGraph");
		drawButton("2d labirinth creation");
		drawButton("EXIT");
	};

	void drawGraphsVisualisation() {
		drawGraph_1();
		drawButton("Menu");
	};

	std::future<void> fut;
	bool visualisation_func_lunched = false;
	void drawLabirinthVisualisation()
	{
		drawButton("Menu");
		drawButton("DFS_recurcive");
		drawButton("DFS_stack");
		drawButton("BFS");
		drawButton("Random");
		drawButton("Clear");

		drawLabirinthField();
		drawLabirinthNet();

		if (ltask_selected == labirinth_task::DFS_recurcive)
		{
			if (!visualisation_func_lunched) {
				fut = std::async(std::launch::async, [&] {labirint_graph.DFS_recurcive(0, visualisation_func_update); });
				visualisation_func_lunched = true;
			}
		}
		else if (ltask_selected == labirinth_task::DFS_stack)
		{
			if (!visualisation_func_lunched) {
				fut = std::async(std::launch::async, [&] {labirint_graph.DFS_stack_2(0, visualisation_func_update); });
				visualisation_func_lunched = true;
			}

		}
		else if (ltask_selected == labirinth_task::BFS)
		{
			if (!visualisation_func_lunched) {
				fut = std::async(std::launch::async, [&] {labirint_graph.BFS(0, visualisation_func_update); });
				visualisation_func_lunched = true;
			}

		}
		else if (ltask_selected == labirinth_task::Random)
		{
			if (!visualisation_func_lunched) {
				fut = std::async(std::launch::async, [&] {labirint_graph.DFS_stack_2_random(FIELD_SIZE_X* FIELD_SIZE_Y/2+ FIELD_SIZE_X/2, visualisation_func_update_colors); });
				visualisation_func_lunched = true;
			}

		}
		else if (ltask_selected == labirinth_task::Clear)
		{
			if (fut.valid()) {
				fut.get();
				for (auto& x : labirinth_)x.color = olc::WHITE;
				ltask_selected = labirinth_task::NONE;
				visualisation_func_lunched = false;
			}
		}
	};

public:
	GraphsVisualisation()
	{
		sAppName = "GraphsVisualisation";
		createLabirinth(FIELD_SIZE_X, FIELD_SIZE_Y);
	}

public:
	bool OnUserCreate() override
	{
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (task_selected == task::welcome_screen)
		{
			Clear(olc::BLACK);
			drawWelcomeScreen();
			mouseControlling();
		}
		else if(task_selected == task::graph_visualisation)
		{
			Clear(olc::BLACK);
			drawGraphsVisualisation();
			mouseControlling();
		}
		else if (task_selected == task::labirinth_visualisation)
		{
			Clear(olc::BLACK);
			drawLabirinthVisualisation();
			mouseControlling();
		}
		
		return true;
	}
};


int main()
{
	GraphsVisualisation inst;
	if (inst.Construct(BUTTON_WIDTH+ SCREEN_SIZE_X, SCREEN_SIZE_Y, 1, 1))
		inst.Start();

	return 0;
}
