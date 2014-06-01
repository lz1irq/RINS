#include "Machine.h"

bool Machine::addItem(Item& i){
	if(items.size() < MAX_ITEMS) {
		items.push_back(&i);
		it = items.end();
		--it;
		return true;
	}
	return false;
}

Item& Machine::getNextItem() {
	++it;
	if(it == items.end()) it = items.begin();
	return *(*it);
}

void Machine::removeItem(int item) {
	list<Item*>::iterator tit = items.begin();
	for(int i=0;i<item;++i) {
		++tit;
	}
	Item& torm = *(*tit);
	items.erase(tit);
	it = items.end();
	--it;
}

Machine::Machine(bool mpaid): paid(mpaid) {} 

int Machine::itemCount() {
	return items.size();
}

bool Machine::isPaid() {
	return paid;
}

int MachineResources::bg = 0;
int MachineResources::frame = 0;
int MachineResources::frame_sel = 0;


MachineManager::MachineManager(Game& mgame, Renderer& mrend, Hitbox& mbox, int mfont): 
over_machine(false), rend(mrend), 
font(mfont), curr_machine(nullptr),
game(mgame), box(mbox), render_machine(false)
{}

bool MachineManager::mouseOverTile(double deltax, double deltay, int tx, int ty){
	int mx = ((game.getMouseX() + deltax) / box.getStepX());
	double msx = mx*box.getStepX();
	int my = ((game.getMouseY() + deltay) / box.getStepY());
	double msy = my*box.getStepY();
	box.setX(msx);
	box.setY(msy);
	if(box.getTileX() == tx && box.getTileY()-1 == ty)return true;
	return false;
}

bool MachineManager::isRendering() {
	return render_machine;
}

bool MachineManager::addItem(pair<int, int> mach, Item& it) {
	return machines.at(mach)->addItem(it);
}

void MachineManager::render() {
	if(render_machine) {
		rend.applyTexture(MachineResources::bg, xstart, ystart, 0.48,0.85);
		int itemc = curr_machine->itemCount();
		int rows = -1;
		for(int i=0;i<itemc;++i) {
			if(i%3 == 0) ++rows;

			Item& it = curr_machine->getNextItem();
			int tid = ItemResources::getTextureID(&typeid(it));

			double xp = 0.027+(i%3)*itemx+framesp;
			double yp = rows*itemy + 5*framesp;

			double nextx = xstart+0.027+((i+1)%3)*itemx+framesp;
			if((i+1)%3 == 0 && i>0) nextx+=3*itemx;
			double nexty = (rows+1)*itemy+5*framesp;
			if(itemsel == i) rend.applyTexture(MachineResources::frame_sel, xp, yp, itemx, itemy);
			else rend.applyTexture(MachineResources::frame, xp, yp, itemx, itemy);
			rend.applyTexture(tid, xp, yp, itemx, itemy);
		}
	}
}
void MachineManager::control(Being* player) {
	if(render_machine) {
		int itemc = curr_machine->itemCount();
		int rows = -1;
		for(int i=0;i<itemc;++i) {
			if(i%3 == 0) ++rows;
			Item& it = curr_machine->getNextItem();
			double xp = 0.024+(i%3)*itemx+framesp;
			double yp = rows*itemy + 5*framesp;

			double nextx = 0.024+((i+1)%3)*itemx+framesp;
			if((i+1)%3 == 0 && i>0) nextx+=3*itemx;
			double nexty = (rows+1)*itemy+5*framesp;
			double mx = game.getMouseX();
			double my = game.getMouseY();

			if(itemsel == -1) {
				if(mx>xp && mx<nextx && my>yp && my<nexty && pressed) itemsel = i;
			 }
			if(itemsel == i) {
				if(cangetpress){
					if(mx>xp && mx<nextx && my>yp && my<nexty) {
						if(curr_machine->isPaid()) {
							if(player->buyItem(it)) {
								curr_machine->removeItem(i);
								cout << "Player bought " << it.getName() << endl;
							}
							else cout << "Not enough money to buy " << it.getName() << endl;
						}
						else {
							if(player->addItem(it)) {
								curr_machine->removeItem(i);
								cout << "Player looted " << it.getName() << endl;
							}
						}
					 }   
					itemsel = -1;
				}
			}
		}
	}
}

bool MachineManager::currentIsPaid() {
	if(curr_machine) return curr_machine->isPaid();
	return false;
}

void MachineManager::check(double dx, double dy, int x, int y) {
	if(curr_machine) {
		if(!over_machine) {
			if(pressed) {
				if(mouseOverTile(dx, dy, curr_x,curr_y)) over_machine = true;
			}
		}
		else {
			if (cangetpress){
				over_machine = false;
				if (mouseOverTile(dx, dy, curr_x, curr_y)) render_machine = true;
				else curr_machine = nullptr;
			}
		}
	}
}

/*
trying to get additem to work when returning item from player inventory
*/
pair<int, int> MachineManager::getCurrentCoords() {
	return make_pair(curr_x, curr_y);
}

void MachineManager::set(pair<int,int> mach) {
	curr_x = mach.first;
	curr_y = mach.second;
	curr_machine = machines.at(mach);
}

void MachineManager::unset() {
	curr_x = -1;
	curr_y = -1;
	curr_machine = 0;
	render_machine = false;
}

bool MachineManager::exists(pair<int, int> mach) {
	if(machines.find(mach) != machines.end()) return true;
	return false;
}

void MachineManager::add(pair<int, int> mach, bool paid) {
	this->machines[mach] = new Machine(paid);
}

void MachineManager::updateVars(double udeltax, double udeltay, bool upressed, bool ucangetpress) {
	deltax = udeltax;
	deltay = udeltay;
	pressed = upressed;
	cangetpress = ucangetpress;
}

void MachineManager::clear() {
	return;
}

