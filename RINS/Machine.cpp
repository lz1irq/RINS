#include "Machine.h"

void Machine::addItem(Item& i){
	items.push_back(&i);
	it = items.end();
}

Item& Machine::getNextItem() {
	++it;
	if(it == items.end()) it = items.begin();
	return *(*it);
}

int Machine::itemCount() {
	return items.size();
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

void MachineManager::addItem(pair<int, int> mach, Item& it) {
	machine.lock();
	machines.at(mach)->addItem(it);
	machine.unlock();
}

void MachineManager::render() {
	machine.lock();
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
	machine.unlock();
}
void MachineManager::control(Being* player) {
	machine.lock();
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

			cout << mx << " " << my << endl;

			if(itemsel == -1) {
				if(mx>xp && mx<nextx && my>yp && my<nexty && pressed) itemsel = i;
			 }
			if(itemsel == i) {
				if(cangetpress){
					if(mx>xp && mx<nextx && my>yp && my<nexty) {
						if(player->buyItem(it)) cout << "Player bought " << it.getName() << endl;
						else cout << "Not enough money to buy " << it.getName() << endl;
						itemsel = -1;
					 }
					else itemsel = -1;
				}
			}
		}
	}
	machine.unlock();
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
				if (mouseOverTile(dx, dy, curr_x, curr_y)) {
					render_machine = true;
				}
				else curr_machine = nullptr;
			}
		}
	}
}

void MachineManager::set(pair<int,int> mach) {
	machine.lock();
	curr_x = mach.first;
	curr_y = mach.second;
	curr_machine = machines.at(mach);
	machine.unlock();
}

void MachineManager::unset() {
	machine.lock();
	curr_x = -1;
	curr_y = -1;
	curr_machine = 0;
	render_machine = false;
	machine.unlock();
}

bool MachineManager::exists(pair<int, int> mach) {
	if(machines.find(mach) != machines.end()) return true;
	return false;
}

void MachineManager::add(pair<int, int> mach) {
	machine.lock();
	this->machines[mach] = new Machine();
	machine.unlock();

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

