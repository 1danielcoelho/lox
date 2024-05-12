class CoffeeMaker {
  init(coffee) {
    this.coffee = coffee;
  }

  brew() {
    print "Enjoy your cup of " + this.coffee;

    // No reusing the grounds!
    this.coffee = "stale";
  }
}

var maker = CoffeeMaker("coffee and chicory");
maker.brew();
maker.brew();