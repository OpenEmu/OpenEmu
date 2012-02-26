#ifdef SA1_CPP

void SA1::serialize(serializer &s) {
  Processor::serialize(s);
  CPUcore::core_serialize(s);

  //sa1.hpp
  s.integer(status.tick_counter);

  s.integer(status.interrupt_pending);

  s.integer(status.scanlines);
  s.integer(status.vcounter);
  s.integer(status.hcounter);

  //bus/bus.hpp
  s.array(iram.data(), iram.size());

  s.integer(cpubwram.dma);

  //dma/dma.hpp
  s.integer(dma.line);

  //mmio/mmio.hpp
  s.integer(mmio.sa1_irq);
  s.integer(mmio.sa1_rdyb);
  s.integer(mmio.sa1_resb);
  s.integer(mmio.sa1_nmi);
  s.integer(mmio.smeg);

  s.integer(mmio.cpu_irqen);
  s.integer(mmio.chdma_irqen);

  s.integer(mmio.cpu_irqcl);
  s.integer(mmio.chdma_irqcl);

  s.integer(mmio.crv);

  s.integer(mmio.cnv);

  s.integer(mmio.civ);

  s.integer(mmio.cpu_irq);
  s.integer(mmio.cpu_ivsw);
  s.integer(mmio.cpu_nvsw);
  s.integer(mmio.cmeg);

  s.integer(mmio.sa1_irqen);
  s.integer(mmio.timer_irqen);
  s.integer(mmio.dma_irqen);
  s.integer(mmio.sa1_nmien);

  s.integer(mmio.sa1_irqcl);
  s.integer(mmio.timer_irqcl);
  s.integer(mmio.dma_irqcl);
  s.integer(mmio.sa1_nmicl);

  s.integer(mmio.snv);

  s.integer(mmio.siv);

  s.integer(mmio.hvselb);
  s.integer(mmio.ven);
  s.integer(mmio.hen);

  s.integer(mmio.hcnt);

  s.integer(mmio.vcnt);

  s.integer(mmio.cbmode);
  s.integer(mmio.cb);

  s.integer(mmio.dbmode);
  s.integer(mmio.db);

  s.integer(mmio.ebmode);
  s.integer(mmio.eb);

  s.integer(mmio.fbmode);
  s.integer(mmio.fb);

  s.integer(mmio.sbm);

  s.integer(mmio.sw46);
  s.integer(mmio.cbm);

  s.integer(mmio.swen);

  s.integer(mmio.cwen);

  s.integer(mmio.bwp);

  s.integer(mmio.siwp);

  s.integer(mmio.ciwp);

  s.integer(mmio.dmaen);
  s.integer(mmio.dprio);
  s.integer(mmio.cden);
  s.integer(mmio.cdsel);
  s.integer(mmio.dd);
  s.integer(mmio.sd);

  s.integer(mmio.chdend);
  s.integer(mmio.dmasize);
  s.integer(mmio.dmacb);

  s.integer(mmio.dsa);

  s.integer(mmio.dda);

  s.integer(mmio.dtc);

  s.integer(mmio.bbf);

  s.array(mmio.brf);

  s.integer(mmio.acm);
  s.integer(mmio.md);

  s.integer(mmio.ma);

  s.integer(mmio.mb);

  s.integer(mmio.hl);
  s.integer(mmio.vb);

  s.integer(mmio.va);
  s.integer(mmio.vbit);

  s.integer(mmio.cpu_irqfl);
  s.integer(mmio.chdma_irqfl);

  s.integer(mmio.sa1_irqfl);
  s.integer(mmio.timer_irqfl);
  s.integer(mmio.dma_irqfl);
  s.integer(mmio.sa1_nmifl);

  s.integer(mmio.hcr);

  s.integer(mmio.vcr);

  s.integer(mmio.mr);

  s.integer(mmio.overflow);
}

#endif
