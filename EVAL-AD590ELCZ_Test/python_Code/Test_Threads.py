from queue import Queue
from threading import Thread
import time

# define some queues
fun2_q = Queue()
fun3_q = Queue()

class myThread(Thread):
    def __init__(self, identifier):
        super(myThread, self).__init__() 

    def fun1(self):
        print('starting fun1')

        # broadcasts messages
        fun2_q.put('say something')
        fun3_q.put('say something')

        fun2_q.put('quit')
        fun3_q.put('quit')



    def fun2(self):
        # event that occurs when a message arrives
        # as a listener we should use infinite loop to monitor messages 
        # we will use non blocking way to read the queue using "if", also we can use fun2_q.get_nowait()
        # instead of "if fun2_q.qsize() > 0:" statement

        while True:
            if fun2_q.qsize() > 0:
                msg = fun2_q.get()
                if msg == 'say something':
                    print('fun2 method saying hello')
                elif msg == 'quit':
                    break  # quit thread

            # do other stuff below if no messages coming

            time.sleep(0.1)  # to stop while loop from abusing processor

        print('fun2 terminating')


    def fun3(self):
        # event that occurs when a message arrives
        # we will use a blocking way to read the queue
        while True:

            msg = fun3_q.get() # it will block here waiting for a message to come
            if msg == 'say something':
                print('fun3 method saying hello')
            elif msg == 'quit':
                    break  # quit thread

            # can't do other stuff below if no messages coming, the loop will stuck waiting new message

            # time.sleep(0.1)  # no need for it since the loop will wait anyway

        print('fun3 terminating')

    def run(self):
        t1 = Thread(target = self.fun1)
        t2 = Thread(target = self.fun2)
        t3 = Thread(target = self.fun3)
        t1.start()
        t2.start()
        t3.start()

my_thread = myThread(1)
my_thread.run()
