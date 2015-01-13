$(function() {
	// Globals
	var GAME_HEIGHT = 600;
	var GAME_WIDTH = 600;
	var TOTAL_STARS = 65;
	var GAME_SPEED = 16;
	var bonusOnScreen = [];
	var stars = [];
	var ship_weapons_fired = [];
	var enemies = [];
	var enemy_weapons_fired = [];
	var STAR_SIZE = 6;
	var SLOW_STAR_SPEED = 1;
	var FAST_STAR_SPEED = 12;
	var STAR_SPEED = SLOW_STAR_SPEED;
	var NORMAL_STAR_INTERVAL = 9;
	var TRANSITION_STAR_INTERVAL = 1;
	var STAR_CREATE_INTERVAL = NORMAL_STAR_INTERVAL;
	var STAR_TIMER = 0;
	var LEFT = 37;
	var RIGHT = 39;
	var SHOOT = 32 ;
	var MOVE_LEFT = 0;
	var MOVE_RIGHT = 1;
	var MOVE_DOWN = 2;
	var MOVE_UP = 3;
	var NO_DIRECTION = 5;
	var DIRECTION;
	var MOVING = false;
	var GOING_RIGHT = false;
	var GOING_LEFT = false;
	var GAME_TIMER;
	var gameover = false;
	var wewon = false;
	var NOT_SHOOTING = true;
	var TRANSITIONING = false;
	var SHIP_PAUSE = 0;
	var WAIT_FOR_TRANSITION = 100;
	var SHOW_MESSAGE = false;

	// Get canvas context
	var c = document.getElementById('thecanvas');
	var canvas = c.getContext('2d');

	// create stars
	function createStars() {

		stars = []; // reset stars array
		for (var i = 0; i < TOTAL_STARS; i++) {

			var x = Math.floor(Math.random() * GAME_WIDTH);
			var y = Math.floor(Math.random() * GAME_HEIGHT);
			stars.push(new Star(x,y));
		}
	}

	// Define stars and how to draw them
	function Star(x,y) {

		this.x = x;
		this.y = y;

		this.draw = function() {

			canvas.beginPath();
			canvas.strokeStyle = '#F2F7C3';
			canvas.lineWidth = '2';
			canvas.moveTo(this.x,this.y);
			canvas.lineTo(this.x,this.y+STAR_SIZE);
			canvas.stroke();

			canvas.beginPath();
			canvas.moveTo(this.x-(STAR_SIZE/2),this.y+(STAR_SIZE/2));
			canvas.lineTo(this.x+(STAR_SIZE/2),this.y+(STAR_SIZE/2));
			canvas.stroke();
		}

		this.move = function() {

			this.y += STAR_SPEED;
			if (this.y > GAME_HEIGHT + STAR_SIZE)
				this.offScreen = true;
		}

		this.offScreen = false;
	}

	// Move stars
	function moveStars() {

		var visibleStars = [];
		for (var i = 0; i < stars.length; i++) {
			stars[i].move();
			if (!stars[i].offScreen)
				visibleStars.push(stars[i]);
		}

		stars = visibleStars;
	}

	// Detect when enemies are hit
	function detectEnemiesHit() {

		// Go through each bullet, see if it collides with an enemy
		for (var i = 0; i < ship_weapons_fired.length; i++) {
			ship_weapons_fired[i].checkCollisions();
		}

		// Remove fired weapons and enemies necessary
		var tempShots = [];
		for (var i = 0; i < ship_weapons_fired.length; i++) {
			if (!ship_weapons_fired[i].offScreen)
				tempShots.push(ship_weapons_fired[i]);
		}
		ship_weapons_fired = tempShots;

		var tempEnemies = [];
		for (var i = 0; i < enemies.length; i ++) {
			if (!enemies[i].offScreen)
				tempEnemies.push(enemies[i]);
		}
		enemies = tempEnemies;
	}

	function enemiesFireLevel2() {

		var LVL2FIRECHANCE = 65;
		if (enemies.length > 0) {

			var totalEnemies = enemies.length;
			var whichenemy = Math.floor(Math.random() * totalEnemies);

			// Roll the dice to see if he fires
			var diceroll = Math.floor(Math.random() * LVL2FIRECHANCE);
			if (diceroll == 0) {
				// fire
				enemies[whichenemy].fire();
			}
		}
	}

	function enemiesFireLevel3() {

		var LVL3FIRECHANCE = 40;
		if (enemies.length > 0) {

			var totalEnemies = enemies.length;
			var whichenemy = Math.floor(Math.random() * totalEnemies);

			// Roll the dice to see if he fires
			var diceroll = Math.floor(Math.random() * LVL3FIRECHANCE);
			if (diceroll == 0) {
				// fire
				enemies[whichenemy].fire();
			}
		}
	}

	function enemiesFireLevel4() {

		var LVL4FIRECHANCE = 25;
		if (enemies.length > 0) {

			var totalEnemies = enemies.length;
			var whichenemy = Math.floor(Math.random() * totalEnemies);

			// Roll the dice to see if he fires
			var diceroll = Math.floor(Math.random() * LVL4FIRECHANCE);
			if (diceroll == 0) {
				// fire
				enemies[whichenemy].fire();
			}
		}
	}

	// Define basic enemy grunt
	function Grunt() {

		// private
		var width = 20;
		var length = 20;
		var speed = 2;
		var direction = MOVE_RIGHT;
		var movingdown = 0;
		var TOMOVEDOWN = 30;

		this.x = -(width);
		this.y = 0;

		this.move = function() {

			if (direction == MOVE_RIGHT) {

				if (this.x + speed + (width/2) > GAME_WIDTH) {
					direction = MOVE_DOWN;
					this.move();
				} else {
					this.x += speed;
				}
			} else if (direction == MOVE_LEFT) {

				if (this.x - speed - (width/2) < 0) {
					direction = MOVE_DOWN;
					this.move();
				} else {
					this.x -= speed;
				}
			} else if (direction == MOVE_DOWN) {

				if (speed + movingdown > TOMOVEDOWN) {
					movingdown = 0;
					if (this.x < GAME_WIDTH/2) {
						direction = MOVE_RIGHT;
						this.move();
					} else {
						direction = MOVE_LEFT;
						this.move();
					}
				} else {
					this.y += speed;
					movingdown += speed;
				}
			}

			if (this.y + length > ship.y)
				gameover = true;
		}

		this.draw = function() {

			// Draw body
			canvas.beginPath();
			canvas.lineWidth = '1';
			canvas.fillStyle = '#666666';
			canvas.strokeStyle = '#eeeeee';
			canvas.fillRect(this.x - (width/2), this.y, width, length);
			canvas.strokeRect(this.x - (width/2), this.y, width, length);

			// Draw face
			canvas.beginPath();
			canvas.strokeStyle = '#333';
			canvas.lineWidth = '2';
			canvas.moveTo(this.x - width/2 + width/5, this.y + length/4);
			canvas.lineTo(this.x - width/2 + (width/5)*2, this.y + length/3);
			canvas.stroke();
			canvas.beginPath();
			canvas.moveTo(this.x + width/2 - width/5, this.y + length/4);
			canvas.lineTo(this.x + width/2 - (width/5)*2, this.y + length/3);
			canvas.stroke();
			canvas.beginPath();
			canvas.moveTo(this.x - width/2 + width/5, this.y + (length/4)*3);
			canvas.lineTo(this.x + width/2 - width/5, this.y + length/2);
			canvas.stroke();
		}

		this.checkIfHitBy = function(shot) {

			var hit = false;
			if (shot.x > this.x - (width/2) && shot.x < this.x + (width/2) || shot.x + shot.getWidth() > this.x - (width/2) && shot.x + shot.getWidth() < this.x + (width/2)) { // In width

				if (shot.y < this.y + length && shot.y > this.y || shot.y + shot.getLength() < this.y + length && shot.y + shot.getLength() > this.y) {
					hit = true;
				}
			}

			return hit;
		}

		this.fire = function() {

			enemy_weapons_fired.push(new EnemyBomb(this.x, this.y + length));
		}

		this.offScreen = false;
	}

	// Define EnemyBomb
	function EnemyBomb(x,y) {

		// private
		var speed = 3;
		var radius = 5;

		// public
		this.x = x;
		this.y = y;
		this.offScreen = false;

		this.move = function() {

			this.y += speed;
			if (this.y + radius > GAME_HEIGHT)
				this.offScreen = true;
		}

		this.draw = function() {

			canvas.beginPath();
			canvas.lineWidth = '1';
			canvas.fillStyle = '#006e2e';
			canvas.strokeStyle = '#eeeeee';
			canvas.arc(this.x, this.y, radius, -(0.5) * Math.PI, 1.5 * Math.PI);
			canvas.fill();
			canvas.stroke();
		}

		this.getLength = function() {
			return radius;
		}

		this.getWidth = function() {
			return radius;
		}

	}

	// Define Rockets
	function Rocket(x,y) {

		var length = 12;
		var width = 6;
		var speed = 6;
		var tipRadius = 4;

		this.x = x;
		this.y = y;
		this.power = 5;

		this.draw = function() {

			canvas.beginPath();
			canvas.lineWidth = '1';
			canvas.fillStyle = '#aaa';
			canvas.strokeStyle = '#eee';
			canvas.fillRect(this.x, this.y, width, length);
			canvas.strokeRect(this.x, this.y, width, length);
			canvas.beginPath();
			canvas.moveTo(this.x, this.y);
			canvas.arc(this.x + width/2, this.y, tipRadius, 0, -(1)*Math.PI, true);
			canvas.fillStyle = 'red';
			canvas.fill();
			canvas.stroke();

		}

		this.move = function() {

			this.y -= speed;
			if (this.y + speed < 0) {
				this.offScreen = true;
			}
		}

		this.checkCollisions = function() {

			// Check each enemy, see if we hit one
			for (var i = 0; i < enemies.length; i++) {

				if (enemies[i].checkIfHitBy(this)) {
					enemies[i].offScreen = true;
					this.offScreen = true;
				}
			}
		}

		this.getLength = function() {
			return length;
		}

		this.getWidth = function() {
			return width;
		}

		this.offScreen = false;
	}

	// Define Missle
	function Missle(x,y) {

		// private
		var length = 8;
		var width = 3;
		var speed = 5;

		// public
		this.x = x;
		this.y = y;
		this.power = 3;

		this.draw = function() {

			canvas.beginPath();
			canvas.moveTo(this.x, this.y);
			canvas.lineWidth = width;
			canvas.strokeStyle = '#ff1a00';
			canvas.lineTo(this.x, this.y - length);
			canvas.stroke();
		}

		this.move = function() {

			this.y -= speed;
			if (this.y + speed < 0) {
				this.offScreen = true;
			}
		}

		this.checkCollisions = function() {

			// Check each enemy, see if we hit one
			for (var i = 0; i < enemies.length; i++) {

				if (enemies[i].checkIfHitBy(this)) {
					enemies[i].offScreen = true;
					this.offScreen = true;
				}
			}
		}

		this.getLength = function() {
			return length;
		}

		this.getWidth = function() {
			return width;
		}

		this.offScreen = false;
	}

	function WeaponSystem() {

		var RESTOCK_INTERVAL = 8;
		var STOCK_COUNT = 0;
		var width = 10;
		var length = 70;
		var blink = false;
		var blink_interval = 5;
		var blinky = 0;

		this.level = 100;
		this.x = 10;
		this.y = GAME_HEIGHT - length - 10;
		this.needsStocking = false;

		this.restock = function() {

			STOCK_COUNT++;
			if (STOCK_COUNT % RESTOCK_INTERVAL == 0) {

				STOCK_COUNT = 0;
				this.level++;
				if (this.level > 100) {
					this.level = 100;
					this.needsStocking = false;
				}
			}
		}

		this.draw = function() {

			if (!this.needsStocking) {
				canvas.beginPath();
				canvas.lineWidth = 1;
				canvas.fillStyle = '#AAAAAA';
				canvas.strokeStyle = '#eeeeee';
				canvas.fillRect(this.x, this.y, width, length);
				this.setFillStyle();
				if (this.level > 0) {
					canvas.fillRect(this.x, this.y + (length - (length * this.level)/100), width, (length * this.level)/100);
				}
				canvas.strokeRect(this.x, this.y, width, length);
			} else {

				blinky++;
				if (blinky % blink_interval == 0) {
					blinky = 0;
					blink = !blink;
				}

				if (blink) {
					canvas.beginPath();
					canvas.lineWidth = 1;
					canvas.fillStyle = '#AAAAAA';
					canvas.strokeStyle = '#eeeeee';
					canvas.fillRect(this.x, this.y, width, length);
					this.setFillStyle();
					if (this.level > 0) {
						canvas.fillRect(this.x, this.y + (length - (length * this.level)/100), width, (length * this.level)/100);
					}
					canvas.strokeRect(this.x, this.y, width, length);
				}
			}
		}

		this.setFillStyle = function() {

			if (this.level > 50) {
				canvas.fillStyle = "#32771F";
			} else if (this.level > 20) {
				canvas.fillStyle = "#E5733D";
			}else if (this.level <= 20) {
				canvas.fillStyle = "#C11600";
			}
		}

		this.shotFired = function(weapon) {

			this.level -= weapon.power;
			if (this.level < 0) {
				this.needsStocking = true;
			}
		}

	}

	// Define Ship
	function Ship() {

		// private
		var SHIP_HEIGHT = 60; // multiples of 6 only
		var SHIP_WIDTH = (SHIP_HEIGHT/6)*5;
		var SHIP_BUFFER = 20;
		var COCKPIT_DIST = 2;
		var COCKPIT_LENGTH = 25;
		var COCKPIT_WIDTH = 6;
		var FLAME_LENGTH = 20;
		var SHIP_SPEED = 5;
		var NORMAL_HEIGHT = 60;
		var WARP_HEIGHT = 120;

		// public
		this.x = GAME_WIDTH/2 - SHIP_WIDTH/2;
		this.y = GAME_HEIGHT - SHIP_HEIGHT - SHIP_BUFFER;
		this.transitionComplete = false;
		this.hasRockets = false;

		this.draw = function() {

			// Ship outline
			var shipgradient = canvas.createLinearGradient(this.x - (SHIP_WIDTH/2), this.y + (SHIP_HEIGHT/2), this.x + (SHIP_WIDTH/2), this.y + (SHIP_HEIGHT/2));
			shipgradient.addColorStop(0,'#499bea');
			shipgradient.addColorStop(0.45, '#B7B7B7');
			shipgradient.addColorStop(0.55, '#B7B7B7');
			shipgradient.addColorStop(1, '#499bea');
			canvas.fillStyle = shipgradient;
			canvas.lineWidth = '1';
			canvas.beginPath();
			canvas.moveTo(this.x,this.y);
			canvas.lineTo(this.x - (SHIP_WIDTH/4), this.y + (SHIP_HEIGHT/3)*2);
			canvas.lineTo(this.x - (SHIP_WIDTH/2), this.y + (((SHIP_HEIGHT/3)*2) + (SHIP_HEIGHT/9)));
			canvas.lineTo(this.x - (SHIP_WIDTH/2), this.y + SHIP_HEIGHT);
			canvas.lineTo(this.x - (SHIP_WIDTH/6), this.y + (SHIP_HEIGHT/9)*8);
			canvas.lineTo(this.x - (SHIP_WIDTH/21)*3, this.y + (SHIP_HEIGHT/18)*17);
			canvas.lineTo(this.x + (SHIP_WIDTH/21)*3, this.y + (SHIP_HEIGHT/18)*17);
			canvas.lineTo(this.x + (SHIP_WIDTH/6), this.y + (SHIP_HEIGHT/9)*8);
			canvas.lineTo(this.x + (SHIP_WIDTH/2), this.y + SHIP_HEIGHT);
			canvas.lineTo(this.x + (SHIP_WIDTH/2), this.y + (((SHIP_HEIGHT/3)*2) + (SHIP_HEIGHT/9)));
			canvas.lineTo(this.x + (SHIP_WIDTH/4), this.y + (SHIP_HEIGHT/3)*2);
			canvas.lineTo(this.x,this.y);
			canvas.fill();
			canvas.strokeStyle ='#FFFFFF';
			canvas.stroke();

			// Wings
			canvas.fillStyle = '#cc0000';
			canvas.beginPath();
			canvas.moveTo(this.x - (SHIP_WIDTH/4), this.y + (SHIP_HEIGHT/3)*2);
			canvas.lineTo(this.x - (SHIP_WIDTH/2), this.y + (((SHIP_HEIGHT/3)*2) + (SHIP_HEIGHT/9)));
			canvas.lineTo(this.x - (SHIP_WIDTH/2), this.y + SHIP_HEIGHT);
			canvas.lineTo(this.x - (SHIP_WIDTH/6), this.y + (SHIP_HEIGHT/9)*8);
			canvas.lineTo(this.x - (SHIP_WIDTH/4), this.y + (SHIP_HEIGHT/3)*2);
			canvas.fill();
			canvas.beginPath();
			canvas.moveTo(this.x - (SHIP_WIDTH/4), this.y + (SHIP_HEIGHT/3)*2);
			canvas.lineTo(this.x - (SHIP_WIDTH/2), this.y + (((SHIP_HEIGHT/3)*2) + (SHIP_HEIGHT/9)));
			canvas.lineTo(this.x - (SHIP_WIDTH/2), this.y + SHIP_HEIGHT);
			canvas.lineTo(this.x - (SHIP_WIDTH/6), this.y + (SHIP_HEIGHT/9)*8);
			canvas.stroke();
			canvas.beginPath();
			canvas.moveTo(this.x + (SHIP_WIDTH/6), this.y + (SHIP_HEIGHT/9)*8);
			canvas.lineTo(this.x + (SHIP_WIDTH/2), this.y + SHIP_HEIGHT);
			canvas.lineTo(this.x + (SHIP_WIDTH/2), this.y + (((SHIP_HEIGHT/3)*2) + (SHIP_HEIGHT/9)));
			canvas.lineTo(this.x + (SHIP_WIDTH/4), this.y + (SHIP_HEIGHT/3)*2);
			canvas.lineTo(this.x + (SHIP_WIDTH/6), this.y + (SHIP_HEIGHT/9)*8);
			canvas.fill();
			canvas.beginPath();
			canvas.moveTo(this.x + (SHIP_WIDTH/6), this.y + (SHIP_HEIGHT/9)*8);
			canvas.lineTo(this.x + (SHIP_WIDTH/2), this.y + SHIP_HEIGHT);
			canvas.lineTo(this.x + (SHIP_WIDTH/2), this.y + (((SHIP_HEIGHT/3)*2) + (SHIP_HEIGHT/9)));
			canvas.lineTo(this.x + (SHIP_WIDTH/4), this.y + (SHIP_HEIGHT/3)*2);
			canvas.stroke();

			// Cool style lines
			canvas.beginPath();
			canvas.strokeStyle = '#666666';
			canvas.moveTo(this.x,this.y + 2);
			canvas.lineTo(this.x + (SHIP_WIDTH/13), this.y + (SHIP_HEIGHT/3)*2);
			canvas.stroke();
			canvas.beginPath();
			canvas.moveTo(this.x,this.y + 2);
			canvas.lineTo(this.x - (SHIP_WIDTH/13), this.y + (SHIP_HEIGHT/3)*2);
			canvas.stroke();

			// Cockpit
			canvas.beginPath();
			canvas.strokeStyle = "#FFFFFF";
			canvas.fillStyle = "#111111";
			canvas.moveTo(this.x, this.y + (SHIP_HEIGHT/18)*17 - COCKPIT_DIST);
			canvas.quadraticCurveTo(this.x - (SHIP_WIDTH/5), this.y + (SHIP_HEIGHT/18)*17 - COCKPIT_DIST, this.x, this.y + (SHIP_HEIGHT/18)*17 - COCKPIT_DIST - COCKPIT_LENGTH);
			canvas.moveTo(this.x, this.y + (SHIP_HEIGHT/18)*17 - COCKPIT_DIST);
			canvas.quadraticCurveTo(this.x + (SHIP_WIDTH/5), this.y + (SHIP_HEIGHT/18)*17 - COCKPIT_DIST, this.x, this.y + (SHIP_HEIGHT/18)*17 - COCKPIT_DIST - COCKPIT_LENGTH);
			canvas.stroke();
			canvas.fill();

			// Exhaust
			canvas.beginPath();
			var flamegradient = canvas.createRadialGradient(this.x, this.y + (SHIP_HEIGHT/18)*17, FLAME_LENGTH/6, this.x, this.y + (SHIP_HEIGHT/18)*17, FLAME_LENGTH);
			flamegradient.addColorStop(0.88, '#B54444');
			flamegradient.addColorStop(0.74, '#CE6C4E');
			flamegradient.addColorStop(0.53, '#E09A55');
			flamegradient.addColorStop(0.36, '#DBB753');
			flamegradient.addColorStop(0.18, '#E5F95E');
			canvas.fillStyle = flamegradient;
			canvas.stokeStyle = '#000000';
			canvas.moveTo(this.x, this.y + (SHIP_HEIGHT/18)*17);
			if (GOING_LEFT) {
				canvas.quadraticCurveTo(this.x - (SHIP_WIDTH/21)*3, this.y + (SHIP_HEIGHT/18)*17, this.x + (SHIP_WIDTH/21)*3, this.y + (SHIP_HEIGHT/18)*17 + FLAME_LENGTH);
				canvas.moveTo(this.x, this.y + (SHIP_HEIGHT/18)*17);
				canvas.quadraticCurveTo(this.x + (SHIP_WIDTH/21)*3, this.y + (SHIP_HEIGHT/18)*17, this.x + (SHIP_WIDTH/21)*3, this.y + (SHIP_HEIGHT/18)*17 + FLAME_LENGTH);
			} else if (GOING_RIGHT){
				canvas.quadraticCurveTo(this.x - (SHIP_WIDTH/21)*3, this.y + (SHIP_HEIGHT/18)*17, this.x - (SHIP_WIDTH/21)*3, this.y + (SHIP_HEIGHT/18)*17 + FLAME_LENGTH);
				canvas.moveTo(this.x, this.y + (SHIP_HEIGHT/18)*17);
				canvas.quadraticCurveTo(this.x + (SHIP_WIDTH/21)*3, this.y + (SHIP_HEIGHT/18)*17, this.x - (SHIP_WIDTH/21)*3, this.y + (SHIP_HEIGHT/18)*17 + FLAME_LENGTH);
			} else {
				if (!TRANSITIONING) {
					canvas.quadraticCurveTo(this.x - (SHIP_WIDTH/21)*3, this.y + (SHIP_HEIGHT/18)*17, this.x, this.y + (SHIP_HEIGHT/18)*17 + FLAME_LENGTH);
					canvas.moveTo(this.x, this.y + (SHIP_HEIGHT/18)*17);
					canvas.quadraticCurveTo(this.x + (SHIP_WIDTH/21)*3, this.y + (SHIP_HEIGHT/18)*17, this.x, this.y + (SHIP_HEIGHT/18)*17 + FLAME_LENGTH);
				} else {
					canvas.quadraticCurveTo(this.x - (SHIP_WIDTH/21)*3, this.y + (SHIP_HEIGHT/18)*17, this.x, this.y + (SHIP_HEIGHT/18)*17 + FLAME_LENGTH);
					canvas.moveTo(this.x, this.y + (SHIP_HEIGHT/18)*17);
					canvas.quadraticCurveTo(this.x + (SHIP_WIDTH/21)*3, this.y + (SHIP_HEIGHT/18)*17, this.x, this.y + (SHIP_HEIGHT/18)*17 + FLAME_LENGTH * 2);
				}
			}
			canvas.fill();
			canvas.stroke();

		}

		this.move = function() {

			if (MOVING) {

				if (DIRECTION == MOVE_LEFT) {

					if (this.x - SHIP_SPEED > 0)
						this.x -= SHIP_SPEED;

				} else if (DIRECTION == MOVE_RIGHT) {

					if (this.x + SHIP_SPEED < GAME_WIDTH) {
						this.x += SHIP_SPEED;
					}
				}
			}
		}

		this.fire = function() {

			if (NOT_SHOOTING) {
				if (!weaponSystem.needsStocking) {

					if (this.hasRockets) {
						var r = new Rocket(this.x - (SHIP_WIDTH/2) - 1, this.y + ((SHIP_HEIGHT/3)*2) + (SHIP_HEIGHT/9) + 3);
						var s = new Rocket(this.x + (SHIP_WIDTH/2) - 6, this.y + ((SHIP_HEIGHT/3)*2) + (SHIP_HEIGHT/9) + 3);
						weaponSystem.shotFired(r);
						weaponSystem.shotFired(s);
						ship_weapons_fired.push(r);
						ship_weapons_fired.push(s);

					} else {

						var m = new Missle(this.x, this.y);
						weaponSystem.shotFired(m);
						ship_weapons_fired.push(m);
					}
				}
			}
		}

		this.isReallyAHit = function(w) {

			// Find out if w is within the ships bounds

			// Check if we hit hull
			var topPointX = this.x; var topPointY = this.y;
			var leftHullX = this.x - (SHIP_WIDTH/4); var leftHullY = this.y + (SHIP_HEIGHT/3)*2;
			var rightHullX = this.x + (SHIP_WIDTH/4); var rightHullY = this.y + (SHIP_HEIGHT/3)*2;
			var wY = w.y + w.getLength()/2;
			var wLeftX = w.x - w.getWidth()/2;
			var wRightX = w.x + w.getWidth()/2;

			if (((topPointX - rightHullX)*(wY - rightHullY) - (topPointY - rightHullY)*(wRightX - rightHullX)) < 0 && ((topPointX - leftHullX)*(wY - leftHullY) - (topPointY - leftHullY)*(wRightX - leftHullX)) > 0) {
				return true;
			}

			if (((topPointX - rightHullX)*(wY - rightHullY) - (topPointY - rightHullY)*(wRightX - rightHullX)) < 0 && ((topPointX - leftHullX)*(wY - leftHullY) - (topPointY - leftHullY)*(wRightX - leftHullX)) > 0) {
				return true;
			}

			return false;
		}

		this.isHit = function() {

			// Check if enemy weapon hit us
			var hit = false;
			for (var i = 0; i < enemy_weapons_fired.length; i++) {

				// Check general area for speed, then be specific
				var w = enemy_weapons_fired[i];
				if ((w.y > this.y && w.y < this.y + SHIP_HEIGHT) || (w.y + w.getLength() > this.y && w.y + w.getLength() < this.y + SHIP_HEIGHT)) {
					if ((w.x > this.x - SHIP_WIDTH/2 && w.x < this.x + SHIP_WIDTH/2) || (w.x + w.getWidth() > this.x - SHIP_WIDTH/2 && w.x + w.getWidth() < this.x + SHIP_WIDTH/2)) { // in width
						// In general area
						hit = this.isReallyAHit(w);
						if (hit)
							break;
					}
				}

			}

			return hit;
		}

		this.gotBonus = function(w) {

			var hit = false;

			// Check general area for speed, then be specific
			if ((w.y > this.y && w.y < this.y + SHIP_HEIGHT) || (w.y + w.getLength() > this.y && w.y + w.getLength() < this.y + SHIP_HEIGHT)) {
				if ((w.x > this.x - SHIP_WIDTH/2 && w.x < this.x + SHIP_WIDTH/2) || (w.x + w.getWidth() > this.x - SHIP_WIDTH/2 && w.x + w.getWidth() < this.x + SHIP_WIDTH/2)) { // in width
					// In general area
					hit = this.isReallyAHit(w);
				}
			}

			return hit;
		}

		this.transition = function() {

			// returns true when done
			if (!this.transitionComplete) {

				if (DIRECTION == MOVE_UP) {

					if (this.y - SHIP_SPEED > GAME_HEIGHT/2) {
						this.y -= SHIP_SPEED;
					} else {
						DIRECTION = NO_DIRECTION;
					}
				}

				if (DIRECTION == NO_DIRECTION) {

					SHIP_PAUSE++;
					if (SHIP_PAUSE > 100)
						DIRECTION = MOVE_DOWN;
				}

				if (DIRECTION == MOVE_DOWN) {

					if (this.y + SHIP_SPEED < GAME_HEIGHT - SHIP_HEIGHT - SHIP_BUFFER) {
						this.y += SHIP_SPEED;
					} else {
						this.y = GAME_HEIGHT - SHIP_HEIGHT - SHIP_BUFFER;
						this.transitionComplete = true;
					}
				}
			}
		}
	}

	// Drop Bonuses for ship to pick up
	var BONUS_CHANCE = 700;

	// BONUS ITEMS
	var BONUS_MISSLES = 0;
	function dropBonuses() {

		var chance = Math.floor(Math.random() * BONUS_CHANCE);
		if (chance == 0) {

			// Send a bonus down
			var TOTAL_BONUS = 1;
			var bonus = Math.floor(Math.random() * TOTAL_BONUS);
			switch(bonus) {
				case BONUS_MISSLES:
				bonusOnScreen.push(new MissleToken());
				break;
				case TOTAL_BONUS:
				break;
			}
		}
	}

	// MissleToken
	function MissleToken() {

		//private
		var length = 14;
		var width = 14;
		var speed = 7;
		var misslebuffer = 2;

		this.x = Math.floor(Math.random() * GAME_WIDTH);
		this.y = -(length);
		this.missle = new Missle(this.x, this.y + misslebuffer);
		this.offScreen = false;

		this.draw = function() {

			canvas.beginPath();
			canvas.fillStyle = '#eee';
			canvas.strokeStyle = '#666';
			canvas.arc(this.x, this.y, width/2, Math.PI * (-0.5), Math.PI * (1.5));
			canvas.fill();
			canvas.stroke();
			this.missle.draw();
		}

		this.move = function() {

			this.y += speed;
			this.missle.y += speed;
			if (this.y > GAME_HEIGHT)
				this.offScreen = true;
		}

		this.getLength = function() {
			return length;
		}

		this.getWidth = function() {
			return width;
		}

		this.activateBonus = function() {

			// Every bonus item needs this function
			// Create ten second RocketTimer
			// For now just turn on rockets
			ship.hasRockets = true;
		}
	}

	// Next Level Message
	function Message() {

		this.level = 0;
		this.draw = function() {

			canvas.textAlign = "center";
			canvas.font = "30px Arial";
			canvas.fillStyle = "#ffffff";
			canvas.lineWidth = '3';
			canvas.fillText('SETTING COORDINATES TO LEVEL ' + this.level, GAME_WIDTH/2, GAME_HEIGHT/2 - 40);
			canvas.fillStyle = "#CC5124";
			canvas.font = "35px Arial";
			canvas.fillText('ACTIVATING HYPER-DRIVE!', GAME_WIDTH/2, GAME_HEIGHT/2);
		}
	}

	// Draw Background
	function drawOuterSpace() {

		// First paint it black
		canvas.fillStyle = "rgb(0,0,0)";
		canvas.fillRect(0,0,GAME_WIDTH,GAME_HEIGHT);

		// Then paint some stars
		for (var i = 0; i < stars.length; i++) {
			stars[i].draw();
		}

	}

	// Draw shots ship has fired
	function drawShipShots() {

		for (var i = 0; i < ship_weapons_fired.length; i++) {
			ship_weapons_fired[i].draw();
		}
	}

	// Draw enemy shots
	function drawEnemyShipShots() {

		for (var i = 0; i < enemy_weapons_fired.length; i++) {
			enemy_weapons_fired[i].draw();
		}
	}

	// Draw enemies
	function drawEnemies() {

		for (var i = 0; i < enemies.length; i++) {
			enemies[i].draw();
		}
	}

	// Draw everything in view
	function drawAll() {

		drawOuterSpace();
		drawShipShots();
		drawEnemyShipShots();
		drawEnemies();
		for (var i = bonusOnScreen.length - 1; i >= 0; i--) {
			bonusOnScreen[i].draw();
		};
		weaponSystem.draw();
		if (SHOW_MESSAGE) {
			message.draw();
		}
		ship.draw();
	}

	// Create new stars
	function newStar() {

		var x = Math.floor(Math.random() * GAME_WIDTH);
		var y = Math.floor(-(STAR_SIZE));
		stars.push(new Star(x,y));
	}

	// Create new enemies
	function newEnemy() {

		enemies.push(new Grunt());
	}

	// Move enemies
	function moveEnemies() {

		for (var i = 0; i < enemies.length; i++) {
			enemies[i].move();
		}
	}

	// Move ship shots fired
	function moveShipShotsFired() {

		var tempShots = [];
		for (var i = 0; i < ship_weapons_fired.length; i++) {
			ship_weapons_fired[i].move();
			if (!ship_weapons_fired[i].offScreen)
				tempShots.push(ship_weapons_fired[i]);
		}

		ship_weapons_fired = tempShots;
	}

	// Move enemy shots fired
	function moveEnemyShotsFired() {

		var tempShots = [];
		for (var i = 0; i < enemy_weapons_fired.length; i++) {
			enemy_weapons_fired[i].move();
			if (!enemy_weapons_fired[i].offScreen)
				tempShots.push(enemy_weapons_fired[i]);
		}

		enemy_weapons_fired = tempShots;
	}

	// Move all on screen array
	function moveAll() {

		var tempBonus = [];
		for (var i = 0; i < bonusOnScreen.length; i++) {
			bonusOnScreen[i].move();
			if (!bonusOnScreen[i].offScreen)
				tempBonus.push(bonusOnScreen[i]);
		}

		bonusOnScreen = tempBonus;

		// Check if ship hits bonus
		for (var i = 0; i < bonusOnScreen.length; i++) {

			if (ship.gotBonus(bonusOnScreen[i])) {

				bonusOnScreen[i].activateBonus();
				bonusOnScreen[i].offScreen = true;
			}
		}
	}

	// Create Stars for Game
	createStars();

	// Create ship
	var ship = new Ship();
	var weaponSystem = new WeaponSystem();
	var message = new Message();


	// Level 1 stuff
	var enemiesPushed = 0;
	var ENEMY_TIMER = 0;
	var ENEMY_INTERVAL = 20;
	var transitiondelay = 0;
	var wave = 50;
	function runLevel1() {

		moveStars();
		ship.move();
		moveShipShotsFired();
		moveEnemies();
		detectEnemiesHit();
		weaponSystem.restock();
		moveAll();
		drawAll();
		STAR_TIMER++;
		ENEMY_TIMER++;
		if (STAR_TIMER % STAR_CREATE_INTERVAL == 0) {
			newStar();
		}

		if (ENEMY_TIMER % ENEMY_INTERVAL == 0 && enemiesPushed < wave) {
			enemiesPushed++;
			newEnemy();
		}

		if (enemiesPushed == wave && enemies.length == 0) {
			wewon = true;
		}

		if (!gameover && !wewon) {
			GAME_TIMER = setTimeout(runLevel1,GAME_SPEED);
		}

		if (gameover)
			gameOver();

		if (wewon) {

			transitiondelay++;
			if (transitiondelay < WAIT_FOR_TRANSITION) {
				SHOW_MESSAGE = true;
				message.level = 2;
				GAME_TIMER = setTimeout(runLevel1,GAME_SPEED);
			} else {
				transitiondelay = 0;
				clearTimeout(GAME_TIMER);
				setUpLevel(2,50);
			}
		}

	}

	// Set up Levels
	function setUpLevel(num,waveSize) {

		enemiesPushed = 0;
		ENEMY_TIMER = 0;
		wave = waveSize;
		STAR_TIMER = 0;
		gameover = false;
		wewon = false;

		ship.transitionComplete = false;
		TRANSITIONING = true;
		DIRECTION = MOVE_UP;
		SHIP_PAUSE = 0;
		transitionToLevel(num);
	}

	function transitionToLevel(num) {

		STAR_CREATE_INTERVAL = TRANSITION_STAR_INTERVAL;
		STAR_SPEED = FAST_STAR_SPEED;
		moveStars();
		moveShipShotsFired();
		moveEnemyShotsFired();
		ship.transition();
		weaponSystem.restock();
		drawAll();
		STAR_TIMER++;
		if (STAR_TIMER % STAR_CREATE_INTERVAL == 0) {
			newStar();
		}

		if (!ship.transitionComplete)
			GAME_TIMER = setTimeout(function() { transitionToLevel(num); }, GAME_SPEED);
		else {

			STAR_SPEED = SLOW_STAR_SPEED;
			STAR_CREATE_INTERVAL = NORMAL_STAR_INTERVAL;
			TRANSITIONING = false;
			SHOW_MESSAGE = false;
			switch(num) {
				case 2:
				runLevel2();
				break;
				case 3:
				runLevel3();
				break;
				case 4:
				runLevel4();
				break;
			}
		}

	}

	function runLevel2() {

		moveStars();
		ship.move();
		moveShipShotsFired();
		moveEnemyShotsFired();
		moveEnemies();
		detectEnemiesHit();
		enemiesFireLevel2();
		weaponSystem.restock();
		moveAll();
		drawAll();
		STAR_TIMER++;
		ENEMY_TIMER++;
		if (STAR_TIMER % STAR_CREATE_INTERVAL == 0) {
			newStar();
		}

		if (ENEMY_TIMER % ENEMY_INTERVAL == 0 && enemiesPushed < wave) {
			enemiesPushed++;
			newEnemy();
		}

		if (enemiesPushed == wave && enemies.length == 0)
			wewon = true;

		if (ship.isHit()) {
			gameover = true;
		}

		if (!gameover && !wewon) {
			GAME_TIMER = setTimeout(runLevel2, GAME_SPEED);
		}

		if (gameover)
			gameOver();

		if (wewon) {

			transitiondelay++;
			if (transitiondelay < WAIT_FOR_TRANSITION) {
				SHOW_MESSAGE = true;
				message.level = 3;
				GAME_TIMER = setTimeout(runLevel2,GAME_SPEED);
			} else {
				transitiondelay = 0;
				clearTimeout(GAME_TIMER);
				setUpLevel(3,75);
			}
		}
	}

	function runLevel3() {

		moveStars();
		ship.move();
		moveShipShotsFired();
		moveEnemyShotsFired();
		moveEnemies();
		detectEnemiesHit();
		enemiesFireLevel3();
		weaponSystem.restock();
		moveAll();
		drawAll();
		STAR_TIMER++;
		ENEMY_TIMER++;
		if (STAR_TIMER % STAR_CREATE_INTERVAL == 0) {
			newStar();
		}

		if (ENEMY_TIMER % ENEMY_INTERVAL == 0 && enemiesPushed < wave) {
			enemiesPushed++;
			newEnemy();
		}

		if (enemiesPushed == wave && enemies.length == 0)
			wewon = true;

		if (ship.isHit()) {
			gameover = true;
		}

		if (!gameover && !wewon)
			GAME_TIMER = setTimeout(runLevel3,GAME_SPEED);

		if (gameover)
			gameOver();

		if (wewon) {

			transitiondelay++;
			if (transitiondelay < WAIT_FOR_TRANSITION) {
				SHOW_MESSAGE = true;
				message.level = 4;
				GAME_TIMER = setTimeout(runLevel3,GAME_SPEED);
			} else {
				transitiondelay = 0;
				clearTimeout(GAME_TIMER);
				setUpLevel(4,100);
			}
		}
	}

	function runLevel4() {

		moveStars();
		ship.move();
		moveShipShotsFired();
		moveEnemyShotsFired();
		moveEnemies();
		detectEnemiesHit();
		enemiesFireLevel4();
		weaponSystem.restock();
		moveAll();
		dropBonuses();
		drawAll();
		STAR_TIMER++;
		ENEMY_TIMER++;
		if (STAR_TIMER % STAR_CREATE_INTERVAL == 0) {
			newStar();
		}

		if (ENEMY_TIMER % ENEMY_INTERVAL == 0 && enemiesPushed < wave) {
			enemiesPushed++;
			newEnemy();
		}

		if (enemiesPushed == wave && enemies.length == 0)
			wewon = true;

		if (ship.isHit()) {
			gameover = true;
		}

		if (!gameover && !wewon)
			GAME_TIMER = setTimeout(runLevel4,GAME_SPEED);

		if (gameover)
			gameOver();

		if (wewon) {

			//console.log('WINNER');
			endGame();
			// transitiondelay++;
			// if (transitiondelay < WAIT_FOR_TRANSITION) {
			// 	GAME_TIMER = setTimeout(runLevel1,GAME_SPEED);
			// } else {
			// 	transitiondelay = 0;
			// 	clearTimeout(GAME_TIMER);
			// 	setUpLevel(4,100);
			// }
		}
	}

	function endGame() {

		drawOuterSpace();
		ship.draw();
		weaponSystem.draw();
		canvas.font = '40px Arial';
		canvas.fillStyle = '#ffffff';
		canvas.textAlign = "center";
		canvas.fillText('WELL DONE!',GAME_WIDTH/2,GAME_HEIGHT/2);
	}

	function gameOver() {

		drawOuterSpace();
		weaponSystem.draw();
		drawEnemies();
		canvas.font = '40px Arial';
		canvas.fillStyle = 'red';
		canvas.textAlign = 'center';
		canvas.fillText('GAME OVER', GAME_WIDTH/2, GAME_HEIGHT/2);
	}

	// Add listeners
	window.addEventListener('keydown', keyPressed);
	window.addEventListener('keyup', keyLifted);

	function keyPressed(evt) {

		if (!TRANSITIONING) {
			switch(evt.which) {
				case LEFT:
				DIRECTION = MOVE_LEFT;
				MOVING = true;
				GOING_LEFT = true;
				break;
				case RIGHT:
				DIRECTION = MOVE_RIGHT;
				MOVING = true;
				GOING_RIGHT = true;
				break;
				case SHOOT:
				ship.fire();
				NOT_SHOOTING = false;
				break;
			}
		}
	}

	function keyLifted(evt) {

		if (evt.which != SHOOT) {

			if (evt.which == LEFT)
				GOING_LEFT = false;
			if (evt.which == RIGHT)
				GOING_RIGHT = false;

			if (evt.which == LEFT && !GOING_RIGHT)
				MOVING = false;
			else if (evt.which == RIGHT && !GOING_LEFT)
				MOVING = false;
		} else {

			NOT_SHOOTING = true;
		}
	}

	runLevel1();
});