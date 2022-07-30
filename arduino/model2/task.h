#ifndef Task_h
#define Task_h

class Task {
  public:
  Task(unsigned long wait): wait(wait), enabled(1), last(0) {}
  void run(unsigned long time);
  virtual void action();
  protected:
  bool enabled;
  unsigned long wait;
  unsigned long last;
};

class Tasks {
  public:
  Tasks() {}
  ~Tasks();
  void add(Task* task);
  void run();
  private:
  Task** tasks;
  int total;
};

#endif
