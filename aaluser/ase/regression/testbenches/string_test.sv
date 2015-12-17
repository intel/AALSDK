module string_test();

   int startlog;
   int endlog;
   logic str_en;
   string str;     

   logic  clk = 0;
   
   
   ccip_logger ccip_logger
     (
      .clk           (clk),
      .enable_logger (startlog),
      .finish_logger(endlog),
      .log_string_en (str_en),
      .log_string    (str)
      );

   initial forever clk = ~clk;
   
   initial begin      
      $display("Test started");      
      startlog = 0;      
      endlog = 0;
      str_en = 0;      
      @(posedge clk);
      @(posedge clk);
      @(posedge clk);
      @(posedge clk);
      startlog = 1;      
      @(posedge clk);
      @(posedge clk);
      str_en = 1;
      str = "This is my string";      
      @(posedge clk);
      str_en = 0;      
      @(posedge clk);
      endlog =1;
      @(posedge clk);
      @(posedge clk);
      @(posedge clk);
      @(posedge clk);      
      $finish;      
   end
   
endmodule
